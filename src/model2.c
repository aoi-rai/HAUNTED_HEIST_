#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include <GL/glew.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "model2.h"

#define MAX_MODELS 32

// 頂点データ
typedef struct {
    float position[3];
    float normal[3];
} Vertex;

// メッシュ情報
typedef struct {
    GLuint vbo;
    GLuint ebo;
    unsigned int indexCount;
} Mesh;

// モデル情報
typedef struct {
    Mesh *meshes;
    unsigned int meshCount;
    int loaded;
} Model;

static Model models[MAX_MODELS];
static int modelCount = 0;


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
    model->meshes = calloc(
        model->meshCount,
        sizeof(Mesh)
    );

    if (model->meshes == NULL) {
        aiReleaseImport(scene);
        return 0;
    }

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {

        const struct aiMesh *mesh = scene->mMeshes[i];

        // モデルの実際の大きさを確認
        float minX = mesh->mVertices[0].x;
        float maxX = mesh->mVertices[0].x;

        float minY = mesh->mVertices[0].y;
        float maxY = mesh->mVertices[0].y;

        float minZ = mesh->mVertices[0].z;
        float maxZ = mesh->mVertices[0].z;

        for (unsigned int j = 1; j < mesh->mNumVertices; j++) {

            float x = mesh->mVertices[j].x;
            float y = mesh->mVertices[j].y;
            float z = mesh->mVertices[j].z;

            if (x < minX) minX = x;
            if (x > maxX) maxX = x;

            if (y < minY) minY = y;
            if (y > maxY) maxY = y;

            if (z < minZ) minZ = z;
            if (z > maxZ) maxZ = z;
        }

        printf("モデルの寸法 X=%.2f Y=%.2f Z=%.2f\n",
              maxX - minX,
               maxY - minY,
               maxZ - minZ);

        Vertex *vertices = malloc(
            sizeof(Vertex) * mesh->mNumVertices
        );

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

        // 頂点データをコピー
        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {

            vertices[j].position[0] = mesh->mVertices[j].x;
            vertices[j].position[1] = mesh->mVertices[j].y;
            vertices[j].position[2] = mesh->mVertices[j].z;

            vertices[j].normal[0] = mesh->mNormals[j].x;
            vertices[j].normal[1] = mesh->mNormals[j].y;
            vertices[j].normal[2] = mesh->mNormals[j].z;
        }

        // インデックスデータをコピー
        unsigned int index = 0;

        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {

            const struct aiFace *face = &mesh->mFaces[j];

            for (unsigned int k = 0; k < face->mNumIndices; k++) {
                indices[index++] = face->mIndices[k];
            }
        }

        Mesh *target = &model->meshes[i];

        target->indexCount = indexCount;

        // OpenGLバッファ作成
        glGenBuffers(1, &target->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, target->vbo);

        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(Vertex) * mesh->mNumVertices,
            vertices,
            GL_STATIC_DRAW
        );

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
void DrawModelAt(
    int modelID,
    unsigned int shaderProgram,
    float x,
    float y,
    float z,
    float scale
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

    // モデル行列
    float modelMatrix[16] = {
        scale, 0.0f, 0.0f, 0.0f,
        0.0f, scale, 0.0f, 0.0f,
        0.0f, 0.0f, scale, 0.0f,
        x, y, z, 1.0f
    };

    GLint modelLocation =
        glGetUniformLocation(shaderProgram, "model");

    glUniformMatrix4fv(
        modelLocation,
        1,
        GL_FALSE,
        modelMatrix
    );

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    for (unsigned int i = 0; i < model->meshCount; i++) {

        Mesh *mesh = &model->meshes[i];

        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);

        glVertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            (void *)offsetof(Vertex, position)
        );

        glVertexAttribPointer(
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            (void *)offsetof(Vertex, normal)
        );

        glBindBuffer(
            GL_ELEMENT_ARRAY_BUFFER,
            mesh->ebo
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

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
    }

    free(model->meshes);

    model->meshes = NULL;
    model->meshCount = 0;
    model->loaded = 0;
}
