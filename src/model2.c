#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/glew.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

#include "model2.h"

#define MAX_MODELS 32

// 頂点データ
typedef struct {
    float position[3];
    float normal[3];
    float texCoord[2];
} Vertex;

// メッシュ情報
typedef struct {
    GLuint vbo;
    GLuint ebo;
    GLuint textureID;
    unsigned int indexCount;

    float diffuseColor[4];
    int useTexture;
    int alphaCutout;
} Mesh;

// モデル情報
typedef struct {
    Mesh *meshes;
    unsigned int meshCount;
    int loaded;
} Model;

static Model models[MAX_MODELS];
static int modelCount = 0;


// PNG画像をOpenGLテクスチャとして読み込む
static GLuint LoadTexture(const char *filename)
{
    SDL_Surface *loadedSurface = IMG_Load(filename);

    if (loadedSurface == NULL) {
        printf("テクスチャ読み込み失敗: %s (%s)\n",
               filename, IMG_GetError());
        return 0;
    }

    SDL_Surface *surface = SDL_ConvertSurfaceFormat(
        loadedSurface,
        SDL_PIXELFORMAT_RGBA32,
        0
    );

    SDL_FreeSurface(loadedSurface);

    if (surface == NULL) {
        printf("画像形式変換失敗: %s\n", SDL_GetError());
        return 0;
    }

    // OpenGL用に上下を反転
    unsigned char *pixels = malloc(
        (size_t)surface->w * surface->h * 4
    );

    if (pixels == NULL) {
        SDL_FreeSurface(surface);
        return 0;
    }

    if (SDL_MUSTLOCK(surface)) {
        SDL_LockSurface(surface);
    }

    for (int y = 0; y < surface->h; y++) {
        unsigned char *source =
            (unsigned char *)surface->pixels +
            (size_t)y * surface->pitch;

        unsigned char *destination =
            pixels +
            (size_t)(surface->h - 1 - y) * surface->w * 4;

        memcpy(destination, source, (size_t)surface->w * 4);
    }

    if (SDL_MUSTLOCK(surface)) {
        SDL_UnlockSurface(surface);
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        surface->w,
        surface->h,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixels
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_REPEAT
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_REPEAT
    );

    glBindTexture(GL_TEXTURE_2D, 0);

    free(pixels);
    SDL_FreeSurface(surface);

    printf("テクスチャ読み込み成功: %s\n", filename);

    return textureID;
}




// マテリアルからテクスチャを読み込む
static GLuint LoadMaterialTexture(
    const struct aiMaterial *material,
    const char *modelFilename
)
{
    struct aiString materialName;
    GLuint textureID = 0;

    // 自然モデルの場合はマテリアル名に応じてテクスチャを指定
    if (strstr(modelFilename, "models/nature/FBX/") != NULL &&
        aiGetMaterialString(
            material,
            AI_MATKEY_NAME,
            &materialName) == AI_SUCCESS) {

        char filename[512];

        // BirchTree_1 の幹
        if (strcmp(materialName.data, "BirchTree_Bark") == 0) {
            snprintf(
                filename,
                sizeof(filename),
                "models/nature/Textures/PineTree_Bark.png"
            );
        }
        // BirchTree_1 の葉
        else if (strcmp(materialName.data, "BirchTree_Leaves") == 0) {
            snprintf(
                filename,
                sizeof(filename),
                "models/nature/Textures/Bush_Leaves.png"
            );
        }
        // それ以外の自然モデル
        else {
            snprintf(
                filename,
                sizeof(filename),
                "models/nature/Textures/%s.png",
                materialName.data
            );
        }

        printf("自然モデル用テクスチャ検索: %s\n", filename);

        textureID = LoadTexture(filename);

        if (textureID != 0) {
            return textureID;
        }
    }

    // 通常モデルのFBXテクスチャを読み込む
    struct aiString texturePath;

    if (aiGetMaterialTexture(
            material,
            aiTextureType_DIFFUSE,
            0,
            &texturePath,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL) == AI_SUCCESS) {

        printf("FBXテクスチャパス: %s\n", texturePath.data);

        if (texturePath.data[0] != '*') {
            char filename[512];

            if (strchr(texturePath.data, '/') == NULL &&
                strchr(texturePath.data, '\\') == NULL) {

                snprintf(
                    filename,
                    sizeof(filename),
                    "textures/%s",
                    texturePath.data
                );

            } else {
                snprintf(
                    filename,
                    sizeof(filename),
                    "%s",
                    texturePath.data
                );
            }

            textureID = LoadTexture(filename);
        }
    }

    return textureID;
}


// モデル読み込み
int LoadModel(const char *filename)
{
    if (modelCount >= MAX_MODELS) {
        printf("モデル数の上限です\n");
        return 0;
    }

    const struct aiScene *scene = aiImportFile(
        filename,
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_PreTransformVertices
    );

    if (scene == NULL) {
        printf("読み込み失敗: %s\n", aiGetErrorString());
        return 0;
    }

    int modelID = modelCount + 1;
    Model *model = &models[modelCount];

    model->meshCount = scene->mNumMeshes;
    model->meshes = calloc(model->meshCount, sizeof(Mesh));

    if (model->meshes == NULL) {
        aiReleaseImport(scene);
        return 0;
    }

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {

        const struct aiMesh *mesh = scene->mMeshes[i];
        Mesh *target = &model->meshes[i];

        printf("\n--- マテリアル情報 ---\n");

        // 基本色の取得
        target->diffuseColor[0] = 1.0f;
        target->diffuseColor[1] = 1.0f;
        target->diffuseColor[2] = 1.0f;
        target->diffuseColor[3] = 1.0f;

        if (mesh->mMaterialIndex < scene->mNumMaterials) {

            const struct aiMaterial *material =
                scene->mMaterials[mesh->mMaterialIndex];

                struct aiString materialName;

if (aiGetMaterialString(
        material,
        AI_MATKEY_NAME,
        &materialName) == AI_SUCCESS) {

    printf("マテリアル名: %s\n", materialName.data);

    // 葉っぱのマテリアルか判定
    if (strstr(materialName.data, "Leaves") != NULL) {
        target->alphaCutout = 1;
    } else {
        target->alphaCutout = 0;
    }
}

            struct aiColor4D diffuseColor;

            if (aiGetMaterialColor(
                    material,
                    AI_MATKEY_COLOR_DIFFUSE,
                    &diffuseColor) == AI_SUCCESS) {

                target->diffuseColor[0] = diffuseColor.r;
target->diffuseColor[1] = diffuseColor.g;
target->diffuseColor[2] = diffuseColor.b;
target->diffuseColor[3] = 1.0f;
            }

            unsigned int textureCount =
                aiGetMaterialTextureCount(
                    material,
                    aiTextureType_DIFFUSE
                );

            printf("テクスチャ数: %u\n", textureCount);

            target->textureID = LoadMaterialTexture(
    material,
    filename
);
        }

        target->useTexture = (target->textureID != 0);

        printf("UV座標数: %u\n", mesh->mNumUVComponents[0]);

        // 頂点データの作成
        Vertex *vertices = calloc(mesh->mNumVertices, sizeof(Vertex));

        unsigned int indexCount = mesh->mNumFaces * 3;

        unsigned int *indices = malloc(
            sizeof(unsigned int) * indexCount
        );

        if (vertices == NULL || indices == NULL) {
            free(vertices);
            free(indices);
            aiReleaseImport(scene);
            return 0;
        }

        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {

            vertices[j].position[0] = mesh->mVertices[j].x;
            vertices[j].position[1] = mesh->mVertices[j].y;
            vertices[j].position[2] = mesh->mVertices[j].z;

            if (mesh->mNormals != NULL) {
                vertices[j].normal[0] = mesh->mNormals[j].x;
                vertices[j].normal[1] = mesh->mNormals[j].y;
                vertices[j].normal[2] = mesh->mNormals[j].z;
            }

            if (mesh->mTextureCoords[0] != NULL) {
                vertices[j].texCoord[0] =
                    mesh->mTextureCoords[0][j].x;

                vertices[j].texCoord[1] =
                    mesh->mTextureCoords[0][j].y;
            }
        }

        // インデックスデータの作成
        unsigned int index = 0;

        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {

            const struct aiFace *face = &mesh->mFaces[j];

            for (unsigned int k = 0; k < face->mNumIndices; k++) {
                indices[index++] = face->mIndices[k];
            }
        }

        target->indexCount = indexCount;

        // VBO
        glGenBuffers(1, &target->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, target->vbo);

        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(Vertex) * mesh->mNumVertices,
            vertices,
            GL_STATIC_DRAW
        );

        // EBO
        glGenBuffers(1, &target->ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, target->ebo);

        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            sizeof(unsigned int) * indexCount,
            indices,
            GL_STATIC_DRAW
        );

        free(vertices);
        free(indices);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    model->loaded = 1;
    modelCount++;

    aiReleaseImport(scene);

    printf("モデル読み込み成功: %s\n", filename);

    return modelID;
}


// モデル描画
void DrawModelAtRotated(
    int modelID,
    unsigned int shaderProgram,
    float x,
    float y,
    float z,
    float scale,
    float rotationY
)
{
    if (modelID <= 0 || modelID > modelCount) {
        return;
    }

    Model *model = &models[modelID - 1];

    if (!model->loaded) {
        return;
    }

    glUseProgram(shaderProgram);

    float c = cosf(rotationY);
float s = sinf(rotationY);

float modelMatrix[16] = {
    c * scale,  0.0f, -s * scale, 0.0f,
    0.0f,       scale, 0.0f,      0.0f,
    s * scale,  0.0f, c * scale, 0.0f,
    x,          y,     z,          1.0f
};
    GLint modelLocation =
        glGetUniformLocation(shaderProgram, "model");

    glUniformMatrix4fv(
        modelLocation,
        1,
        GL_FALSE,
        modelMatrix
    );

    GLint colorLocation =
        glGetUniformLocation(shaderProgram, "materialColor");

    GLint textureLocation =
        glGetUniformLocation(shaderProgram, "diffuseTexture");

    GLint useTextureLocation =
        glGetUniformLocation(shaderProgram, "useTexture");

        GLint alphaCutoutLocation =
    glGetUniformLocation(shaderProgram, "alphaCutout");

    glUniform1i(textureLocation, 0);

  glDisable(GL_BLEND);
glDepthMask(GL_TRUE);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    for (unsigned int i = 0; i < model->meshCount; i++) {

        Mesh *mesh = &model->meshes[i];

        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);

        glUniform4fv(
            colorLocation,
            1,
            mesh->diffuseColor
        );

        glUniform1i(
            useTextureLocation,
            mesh->useTexture
        );

        glUniform1i(
    alphaCutoutLocation,
    mesh->alphaCutout
);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh->textureID);

        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE,
            sizeof(Vertex),
            (void *)offsetof(Vertex, position)
        );

        glVertexAttribPointer(
            1, 3, GL_FLOAT, GL_FALSE,
            sizeof(Vertex),
            (void *)offsetof(Vertex, normal)
        );

        glVertexAttribPointer(
            2, 2, GL_FLOAT, GL_FALSE,
            sizeof(Vertex),
            (void *)offsetof(Vertex, texCoord)
        );

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);

        glDrawElements(
            GL_TRIANGLES,
            mesh->indexCount,
            GL_UNSIGNED_INT,
            0
        );
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
// ----------------------------------------
// 選択中NPCの緑色の輪郭を描画
// ----------------------------------------
void DrawModelAtRotatedOutline(
    int modelID,
    unsigned int shaderProgram,
    float x,
    float y,
    float z,
    float scale,
    float rotationY
)
{
    if (modelID <= 0 || modelID > modelCount) {
        return;
    }

    Model *model = &models[modelID - 1];

    if (!model->loaded) {
        return;
    }

    glUseProgram(shaderProgram);

    float c = cosf(rotationY);
    float s = sinf(rotationY);

    // 通常モデルより少し大きくする
    float outlineScale = scale * 1.04f;

    float modelMatrix[16] = {
        c * outlineScale, 0.0f, -s * outlineScale, 0.0f,
        0.0f, outlineScale, 0.0f, 0.0f,
        s * outlineScale, 0.0f, c * outlineScale, 0.0f,
        x, y, z, 1.0f
    };

    GLint modelLocation =
        glGetUniformLocation(shaderProgram, "model");

    GLint colorLocation =
        glGetUniformLocation(shaderProgram, "materialColor");

    GLint useTextureLocation =
        glGetUniformLocation(shaderProgram, "useTexture");

    glUniformMatrix4fv(
        modelLocation,
        1,
        GL_FALSE,
        modelMatrix
    );

    // ----------------------------------------
// 輪郭の発光をゆっくり変化させる
// ----------------------------------------
float time = (float)SDL_GetTicks() / 1000.0f;

// 0.0～1.0の間でゆっくり変化
float pulse = (sinf(time * 3.0f) + 1.0f) * 0.5f;

// 明るさを調整
float brightness = 0.55f + pulse * 0.45f;

// 緑色の輪郭
float outlineColor[4] = {
    0.05f * brightness,
    brightness,
    0.15f * brightness,
    1.0f
};

    glUniform4fv(colorLocation, 1, outlineColor);
    glUniform1i(useTextureLocation, 0);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    // 裏側の面だけを描画する
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glDisable(GL_BLEND);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    for (unsigned int i = 0; i < model->meshCount; i++) {

        Mesh *mesh = &model->meshes[i];

        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);

        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE,
            sizeof(Vertex),
            (void *)offsetof(Vertex, position)
        );

        glVertexAttribPointer(
            1, 3, GL_FLOAT, GL_FALSE,
            sizeof(Vertex),
            (void *)offsetof(Vertex, normal)
        );

        glVertexAttribPointer(
            2, 2, GL_FLOAT, GL_FALSE,
            sizeof(Vertex),
            (void *)offsetof(Vertex, texCoord)
        );

        glDrawElements(
            GL_TRIANGLES,
            mesh->indexCount,
            GL_UNSIGNED_INT,
            0
        );
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // OpenGLの状態を元に戻す
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);

    glDepthMask(GL_TRUE);
}

// 回転なしで描画する従来の関数
void DrawModelAt(
    int modelID,
    unsigned int shaderProgram,
    float x,
    float y,
    float z,
    float scale
)
{
    DrawModelAtRotated(
        modelID,
        shaderProgram,
        x,
        y,
        z,
        scale,
        0.0f
    );
}

// モデル解放
void FreeModel(int modelID)
{
    if (modelID <= 0 || modelID > modelCount) {
        return;
    }

    Model *model = &models[modelID - 1];

    for (unsigned int i = 0; i < model->meshCount; i++) {

        glDeleteBuffers(1, &model->meshes[i].vbo);
        glDeleteBuffers(1, &model->meshes[i].ebo);

        if (model->meshes[i].textureID != 0) {
            glDeleteTextures(1, &model->meshes[i].textureID);
        }
    }

    free(model->meshes);

    model->meshes = NULL;
    model->meshCount = 0;
    model->loaded = 0;
}