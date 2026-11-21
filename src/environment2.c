#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <GL/glew.h>

#include "model2.h"
#include "environment2.h"

#define TREE_COUNT 320
#define BUSH_COUNT 24
#define ROCK_COUNT 30
#define FLOWER_COUNT 25

typedef struct {
    float x;
    float z;
    float rotation;
    float scale;
} BushPosition;

typedef struct {
    float x;
    float z;
    float rotation;
    float scale;
    int modelIndex;
} RockPosition;

typedef struct {
    float x;
    float z;
    int type;
    float scale;
    float rotation;
} FlowerPosition;

// モデルID
static int birchTreeID = 0;
static int bushID = 0;
static int rockIDs[5] = {0};
static int grassID = 0;
static int flowerIDs[5] = {0};

// 配置情報
static GrassPosition grasses[GRASS_COUNT];
static TreePosition trees[TREE_COUNT];
static BushPosition bushes[BUSH_COUNT];
static RockPosition rocks[ROCK_COUNT];

static const FlowerPosition flowers[FLOWER_COUNT] = {
    {-37, -34, 0, 0.015f, 0},
    {-29, -38, 1, 0.014f, 45},
    {-20, -35, 2, 0.015f, 90},
    {-11, -38, 3, 0.014f, 180},
    { -2, -35, 4, 0.015f, 270},
    {  8, -37, 0, 0.014f, 30},
    { 17, -34, 1, 0.015f, 120},
    { 27, -37, 2, 0.014f, 210},
    { 36, -30, 3, 0.015f, 300},

    {-37, -15, 4, 0.014f, 60},
    {-38,   5, 0, 0.015f, 150},
    {-36,  20, 1, 0.014f, 240},

    { 37, -15, 2, 0.015f, 330},
    { 38,   5, 3, 0.014f, 90},
    { 36,  20, 4, 0.015f, 180},

    {-34,  34, 0, 0.015f, 270},
    {-25,  37, 1, 0.014f, 45},
    {-15,  35, 2, 0.015f, 135},
    { -5,  38, 3, 0.014f, 225},
    {  5,  35, 4, 0.015f, 315},
    { 15,  38, 0, 0.014f, 60},
    { 25,  35, 1, 0.015f, 150},
    { 35,  34, 2, 0.014f, 240},

    {-30, -30, 3, 0.015f, 30},
    { 30,  30, 4, 0.014f, 120}
};


#define FENCE_MAX_VERTICES 8000

typedef struct {
    float position[3];
    float normal[3];
    float texCoord[2];
} EnvironmentVertex;

static EnvironmentVertex fenceVertices[FENCE_MAX_VERTICES];
static int fenceVertexCount = 0;
static GLuint fenceVBO = 0;

/* 直方体を三角形36頂点として追加する */
static void AddFenceBox(
    float cx, float cy, float cz,
    float hx, float hy, float hz)
{
    static const float corners[6][4][3] = {
        {{-1,-1, 1},{ 1,-1, 1},{ 1, 1, 1},{-1, 1, 1}},
        {{ 1,-1,-1},{-1,-1,-1},{-1, 1,-1},{ 1, 1,-1}},
        {{ 1,-1, 1},{ 1,-1,-1},{ 1, 1,-1},{ 1, 1, 1}},
        {{-1,-1,-1},{-1,-1, 1},{-1, 1, 1},{-1, 1,-1}},
        {{-1, 1, 1},{ 1, 1, 1},{ 1, 1,-1},{-1, 1,-1}},
        {{-1,-1,-1},{ 1,-1,-1},{ 1,-1, 1},{-1,-1, 1}}
    };

    static const float normals[6][3] = {
        {0,0,1}, {0,0,-1}, {1,0,0},
        {-1,0,0}, {0,1,0}, {0,-1,0}
    };

    static const int order[6] = {0,1,2,0,2,3};

    if (fenceVertexCount + 36 > FENCE_MAX_VERTICES) {
        return;
    }

    for (int face = 0; face < 6; face++) {
        for (int v = 0; v < 6; v++) {
            int c = order[v];
            EnvironmentVertex *out =
                &fenceVertices[fenceVertexCount++];

            out->position[0] = cx + corners[face][c][0] * hx;
            out->position[1] = cy + corners[face][c][1] * hy;
            out->position[2] = cz + corners[face][c][2] * hz;

            out->normal[0] = normals[face][0];
            out->normal[1] = normals[face][1];
            out->normal[2] = normals[face][2];

            out->texCoord[0] = 0.0f;
            out->texCoord[1] = 0.0f;
        }
    }
}

/* 森の外周を囲む木製の塀と、南側中央の開いた門 */
static int InitFence(void)
{
    fenceVertexCount = 0;

    /* 北側の柱 */
    for (int i = 0; i <= 39; i++) {
        float x = -58.5f + i * 3.0f;
        AddFenceBox(x, 1.65f, 58.5f, 0.14f, 1.65f, 0.14f);
    }

    /* 南側の柱。中央の門の幅を空ける */
    for (int i = 0; i <= 39; i++) {
        float x = -58.5f + i * 3.0f;
        if (x < -4.0f || x > 4.0f) {
            AddFenceBox(x, 1.65f, -58.5f, 0.14f, 1.65f, 0.14f);
        }
    }

    /* 東西の柱 */
    for (int i = 0; i <= 39; i++) {
        float z = -58.5f + i * 3.0f;
        AddFenceBox(-58.5f, 1.65f, z, 0.14f, 1.65f, 0.14f);
        AddFenceBox( 58.5f, 1.65f, z, 0.14f, 1.65f, 0.14f);
    }

    /* 門柱 */
    AddFenceBox(-4.0f, 1.8f, -58.5f, 0.22f, 1.8f, 0.22f);
    AddFenceBox( 4.0f, 1.8f, -58.5f, 0.22f, 1.8f, 0.22f);

    /* 北側の横木 */
    AddFenceBox(0.0f, 0.9f, 58.5f, 58.5f, 0.13f, 0.13f);
    AddFenceBox(0.0f, 2.35f, 58.5f, 58.5f, 0.13f, 0.13f);

    /* 東西の横木 */
    AddFenceBox(-58.5f, 0.9f, 0.0f, 0.13f, 0.13f, 58.5f);
    AddFenceBox(-58.5f, 2.35f, 0.0f, 0.13f, 0.13f, 58.5f);
    AddFenceBox(58.5f, 0.9f, 0.0f, 0.13f, 0.13f, 58.5f);
    AddFenceBox(58.5f, 2.35f, 0.0f, 0.13f, 0.13f, 58.5f);

    /* 南側の横木。門の左右だけ */
    AddFenceBox(-31.25f, 0.9f, -58.5f, 27.25f, 0.13f, 0.13f);
    AddFenceBox(-31.25f, 2.35f, -58.5f, 27.25f, 0.13f, 0.13f);
    AddFenceBox(31.25f, 0.9f, -58.5f, 27.25f, 0.13f, 0.13f);
    AddFenceBox(31.25f, 2.35f, -58.5f, 27.25f, 0.13f, 0.13f);

    /* 開いた状態の門扉 */
    AddFenceBox(-4.0f, 1.35f, -61.0f, 0.12f, 1.35f, 2.0f);
    AddFenceBox(-4.0f, 0.45f, -61.0f, 0.18f, 0.12f, 2.0f);
    AddFenceBox(-4.0f, 2.25f, -61.0f, 0.18f, 0.12f, 2.0f);

    AddFenceBox(4.0f, 1.35f, -61.0f, 0.12f, 1.35f, 2.0f);
    AddFenceBox(4.0f, 0.45f, -61.0f, 0.18f, 0.12f, 2.0f);
    AddFenceBox(4.0f, 2.25f, -61.0f, 0.18f, 0.12f, 2.0f);

    glGenBuffers(1, &fenceVBO);
    if (fenceVBO == 0) {
        printf("塀の頂点バッファ作成失敗\n");
        return 0;
    }

    glBindBuffer(GL_ARRAY_BUFFER, fenceVBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        (GLsizeiptr)(fenceVertexCount * sizeof(EnvironmentVertex)),
        fenceVertices,
        GL_STATIC_DRAW
    );
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    printf("塀・門の初期化成功: 頂点数 %d\n", fenceVertexCount);
    return 1;
}

static void DrawFence(unsigned int shaderProgram)
{
    if (fenceVBO == 0 || fenceVertexCount <= 0) {
        return;
    }

    glUseProgram(shaderProgram);

    const float identity[16] = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };

    const float woodColor[4] = {
        0.30f, 0.16f, 0.075f, 1.0f
    };

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "materialColor");
    GLint textureLoc = glGetUniformLocation(shaderProgram, "diffuseTexture");
    GLint useTextureLoc = glGetUniformLocation(shaderProgram, "useTexture");
    GLint alphaLoc = glGetUniformLocation(shaderProgram, "alphaCutout");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, identity);
    glUniform4fv(colorLoc, 1, woodColor);
    glUniform1i(textureLoc, 0);
    glUniform1i(useTextureLoc, 0);
    glUniform1i(alphaLoc, 0);

    glBindBuffer(GL_ARRAY_BUFFER, fenceVBO);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, sizeof(EnvironmentVertex),
        (void *)offsetof(EnvironmentVertex, position)
    );
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, sizeof(EnvironmentVertex),
        (void *)offsetof(EnvironmentVertex, normal)
    );
    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE, sizeof(EnvironmentVertex),
        (void *)offsetof(EnvironmentVertex, texCoord)
    );

    glDrawArrays(GL_TRIANGLES, 0, fenceVertexCount);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// 部分的に読み込みに失敗した場合も解放できるようにする
void FreeEnvironment(void)
{
    if (fenceVBO != 0) {
        glDeleteBuffers(1, &fenceVBO);
        fenceVBO = 0;
        fenceVertexCount = 0;
    }

    if (birchTreeID != 0) {
        FreeModel(birchTreeID);
        birchTreeID = 0;
    }

    if (bushID != 0) {
        FreeModel(bushID);
        bushID = 0;
    }

    for (int i = 0; i < 5; i++) {
        if (rockIDs[i] != 0) {
            FreeModel(rockIDs[i]);
            rockIDs[i] = 0;
        }
    }

    if (grassID != 0) {
        FreeModel(grassID);
        grassID = 0;
    }

    for (int i = 0; i < 5; i++) {
        if (flowerIDs[i] != 0) {
            FreeModel(flowerIDs[i]);
            flowerIDs[i] = 0;
        }
    }
}

int InitEnvironment(void)
{
    // 木・低木・岩モデル
    birchTreeID =
        LoadModel("models/nature/FBX/BirchTree_1.fbx");

    printf("木モデルID: %d\n", birchTreeID);

    bushID =
        LoadModel("models/nature/FBX/Bush.fbx");

    rockIDs[0] = LoadModel("models/nature/FBX/Rock_1.fbx");
    rockIDs[1] = LoadModel("models/nature/FBX/Rock_2.fbx");
    rockIDs[2] = LoadModel("models/nature/FBX/Rock_3.fbx");
    rockIDs[3] = LoadModel("models/nature/FBX/Rock_4.fbx");
    rockIDs[4] = LoadModel("models/nature/FBX/Rock_5.fbx");

    // 草モデル
    grassID =
        LoadModel("models/nature/FBX/Grass_Small.fbx");

    // 花モデル
    flowerIDs[0] = LoadModel("models/nature/FBX/Flower_1_Clump.fbx");
    flowerIDs[1] = LoadModel("models/nature/FBX/Flower_2_Clump.fbx");
    flowerIDs[2] = LoadModel("models/nature/FBX/Flower_3_Clump.fbx");
    flowerIDs[3] = LoadModel("models/nature/FBX/Flower_4_Clump.fbx");
    flowerIDs[4] = LoadModel("models/nature/FBX/Flower_5_Clump.fbx");

    // 読み込み確認
    if (birchTreeID == 0 || bushID == 0 || grassID == 0) {
        printf("環境モデル読み込み失敗\n");
        FreeEnvironment();
        return 0;
    }

    for (int i = 0; i < 5; i++) {
        if (rockIDs[i] == 0) {
            printf("岩モデル読み込み失敗: %d\n", i + 1);
            FreeEnvironment();
            return 0;
        }

        if (flowerIDs[i] == 0) {
            printf("花モデル読み込み失敗: %d\n", i + 1);
            FreeEnvironment();
            return 0;
        }
    }

    // 草の配置：森の中に群生するように配置
    static const float grassPatches[12][2] = {
        {-46.0f, -25.0f}, {-46.0f, 0.0f}, {-46.0f, 25.0f},
        { 46.0f, -25.0f}, { 46.0f, 0.0f}, { 46.0f, 25.0f},
        {-25.0f, 48.0f}, {  0.0f, 49.0f}, { 25.0f, 48.0f},
        {-25.0f,-48.0f}, { 25.0f,-48.0f}, {  0.0f, 54.0f}
    };

    for (int i = 0; i < GRASS_COUNT; i++) {
        int patch = rand() % 12;

        float offsetX =
            ((float)rand() / (float)RAND_MAX - 0.5f) * 8.0f;
        float offsetZ =
            ((float)rand() / (float)RAND_MAX - 0.5f) * 8.0f;

        grasses[i].x = grassPatches[patch][0] + offsetX;
        grasses[i].z = grassPatches[patch][1] + offsetZ;

        grasses[i].scale =
            0.020f + ((float)rand() / (float)RAND_MAX) * 0.012f;

        grasses[i].rotation = (float)(rand() % 360);
    }

    // 木の配置（合計120本）
    int treeCount = 0;

    for (int i = 0; i < 30; i++) {
        float z = -35.0f + i * (70.0f / 29.0f);
        z += (float)((i % 3) - 1) * 1.0f;

        float scale = 0.018f + (i % 5) * 0.001f;
        float rotation = i * 0.71f;

        trees[treeCount++] = (TreePosition){
            -37.0f, z, rotation, scale
        };

        trees[treeCount++] = (TreePosition){
            37.0f, z, rotation + 1.2f, scale
        };
    }

    for (int i = 0; i < 30; i++) {
        float x = -35.0f + i * (70.0f / 29.0f);
        x += (float)((i % 3) - 1) * 1.0f;

        float scale = 0.018f + (i % 5) * 0.001f;
        float rotation = i * 0.83f;

        trees[treeCount++] = (TreePosition){
            x, -37.0f, rotation, scale
        };

        trees[treeCount++] = (TreePosition){
            x, 37.0f, rotation + 1.5f, scale
        };
    }


    /* 森の外側に木を56本追加 */
    for (int i = 0; i < 14; i++) {
        float p = -42.0f + i * 6.46f;
        float scale = 0.018f + (i % 4) * 0.001f;
        float rotation = i * 1.17f;

        trees[treeCount++] = (TreePosition){
            -45.0f, p, rotation, scale
        };
        trees[treeCount++] = (TreePosition){
            45.0f, p, rotation + 0.8f, scale
        };
        trees[treeCount++] = (TreePosition){
            p, -45.0f, rotation + 1.4f, scale
        };
        trees[treeCount++] = (TreePosition){
            p, 45.0f, rotation + 2.1f, scale
        };
    }

    /* さらに外側に木を64本追加 */
    for (int i = 0; i < 18; i++) {
        float p = -52.0f + i * (104.0f / 17.0f);

        /* 南側の門の前は木を置かない */
        if (p > -8.0f && p < 8.0f) {
            continue;
        }

        float scale = 0.018f + (i % 5) * 0.001f;
        float rotation = i * 1.31f;

        trees[treeCount++] = (TreePosition){
            -54.5f, p, rotation, scale
        };
        trees[treeCount++] = (TreePosition){
            54.5f, p, rotation + 0.7f, scale
        };
        trees[treeCount++] = (TreePosition){
            p, -54.5f, rotation + 1.4f, scale
        };
        trees[treeCount++] = (TreePosition){
            p, 54.5f, rotation + 2.1f, scale
        };
    }

    /* 森の中間列に木を80本追加 */
    for (int i = 0; i < 20; i++) {
        /* 南側の門の正面を空けるため、左右に分けて配置 */
        float p;
        if (i < 10) {
            p = -49.0f + i * 4.5f;
        } else {
            p = 8.5f + (i - 10) * 4.5f;
        }

        float scale = 0.018f + (i % 5) * 0.001f;
        float rotation = i * 1.23f;

        trees[treeCount++] = (TreePosition){
            -50.5f, p, rotation, scale
        };
        trees[treeCount++] = (TreePosition){
            50.5f, p, rotation + 0.6f, scale
        };
        trees[treeCount++] = (TreePosition){
            p, 50.5f, rotation + 1.2f, scale
        };
        trees[treeCount++] = (TreePosition){
            p, -50.5f, rotation + 1.8f, scale
        };
    }

    // 木の位置・大きさ・向きに自然なばらつきを加える
    for (int i = 0; i < treeCount; i++) {
        float randomX =
            ((float)rand() / (float)RAND_MAX - 0.5f) * 3.0f;
        float randomZ =
            ((float)rand() / (float)RAND_MAX - 0.5f) * 3.0f;

        trees[i].x += randomX;
        trees[i].z += randomZ;

        trees[i].scale *=
            0.85f + ((float)rand() / (float)RAND_MAX) * 0.30f;

        trees[i].rotation = (float)(rand() % 360);
    }

    printf("木の配置数: %d / %d\\n", treeCount, TREE_COUNT);

    // 低木の配置（24か所）
    int bushCount = 0;

    for (int i = 0; i < 6; i++) {
        float z = -32.0f + i * 12.0f;

        bushes[bushCount++] = (BushPosition){
            -32.0f, z, i * 0.7f, 0.012f
        };

        bushes[bushCount++] = (BushPosition){
            32.0f, z + 4.0f, i * 1.1f, 0.012f
        };
    }

    for (int i = 0; i < 6; i++) {
        float x = -32.0f + i * 12.0f;

        bushes[bushCount++] = (BushPosition){
            x, -32.0f, i * 0.8f, 0.012f
        };

        bushes[bushCount++] = (BushPosition){
            x + 4.0f, 32.0f, i * 1.2f, 0.012f
        };
    }

    // 岩の配置（30か所）
    int rockCount = 0;

    for (int i = 0; i < 8; i++) {
        float z = -35.0f + i * 10.0f;

        rocks[rockCount++] = (RockPosition){
            -34.0f, z, i * 0.7f,
            0.010f + (i % 3) * 0.003f,
            i % 5
        };

        rocks[rockCount++] = (RockPosition){
            34.0f, z + 3.0f, i * 1.1f,
            0.010f + (i % 3) * 0.003f,
            (i + 2) % 5
        };
    }

    for (int i = 0; i < 7; i++) {
        float x = -30.0f + i * 10.0f;

        rocks[rockCount++] = (RockPosition){
            x, -34.0f, i * 0.8f,
            0.010f + (i % 3) * 0.003f,
            (i + 1) % 5
        };

        rocks[rockCount++] = (RockPosition){
            x + 3.0f, 34.0f, i * 1.2f,
            0.010f + (i % 3) * 0.003f,
            (i + 3) % 5
        };
    }

    // 低木の位置・大きさ・向きを少しずつ変える
    for (int i = 0; i < bushCount; i++) {
        bushes[i].x +=
            ((float)rand() / (float)RAND_MAX - 0.5f) * 3.0f;
        bushes[i].z +=
            ((float)rand() / (float)RAND_MAX - 0.5f) * 3.0f;
        bushes[i].scale *=
            0.85f + ((float)rand() / (float)RAND_MAX) * 0.30f;
        bushes[i].rotation = (float)(rand() % 360);
    }

    // 岩の位置・大きさ・向きを少しずつ変える
    for (int i = 0; i < rockCount; i++) {
        rocks[i].x +=
            ((float)rand() / (float)RAND_MAX - 0.5f) * 3.0f;
        rocks[i].z +=
            ((float)rand() / (float)RAND_MAX - 0.5f) * 3.0f;
        rocks[i].scale *=
            0.85f + ((float)rand() / (float)RAND_MAX) * 0.30f;
        rocks[i].rotation = (float)(rand() % 360);
    }

    if (!InitFence()) {
        FreeEnvironment();
        return 0;
    }

    return 1;
}


// 円と四角形のXZ平面上での衝突判定
static int CircleIntersectsBox(
    float x, float z, float radius,
    float centerX, float centerZ,
    float halfX, float halfZ
)
{
    float nearestX = x;
    float nearestZ = z;

    if (nearestX < centerX - halfX) {
        nearestX = centerX - halfX;
    } else if (nearestX > centerX + halfX) {
        nearestX = centerX + halfX;
    }

    if (nearestZ < centerZ - halfZ) {
        nearestZ = centerZ - halfZ;
    } else if (nearestZ > centerZ + halfZ) {
        nearestZ = centerZ + halfZ;
    }

    float dx = x - nearestX;
    float dz = z - nearestZ;

    return dx * dx + dz * dz <= radius * radius;
}


// プレイヤーと森の障害物・塀の衝突判定
int CheckForestCollision(float x, float z, float playerRadius)
{
    // 木：幹の下部15％を当たり判定に使う
    float treeRadius = GetModelTrunkRadiusXZ(birchTreeID);

    for (int i = 0; i < TREE_COUNT; i++) {
        float obstacleRadius = treeRadius * trees[i].scale;
        float dx = x - trees[i].x;
        float dz = z - trees[i].z;
        float totalRadius = playerRadius + obstacleRadius;

        if (dx * dx + dz * dz <= totalRadius * totalRadius) {
            return 1;
        }
    }

    // 低木：モデル全体の水平半径を使う
    float bushRadius = GetModelRadiusXZ(bushID);

    for (int i = 0; i < BUSH_COUNT; i++) {
        float obstacleRadius = bushRadius * bushes[i].scale;
        float dx = x - bushes[i].x;
        float dz = z - bushes[i].z;
        float totalRadius = playerRadius + obstacleRadius;

        if (dx * dx + dz * dz <= totalRadius * totalRadius) {
            return 1;
        }
    }

    // 岩：種類ごとのモデル水平半径を使う
    for (int i = 0; i < ROCK_COUNT; i++) {
        int modelID = rockIDs[rocks[i].modelIndex];
        float obstacleRadius = GetModelRadiusXZ(modelID) * rocks[i].scale;
        float dx = x - rocks[i].x;
        float dz = z - rocks[i].z;
        float totalRadius = playerRadius + obstacleRadius;

        if (dx * dx + dz * dz <= totalRadius * totalRadius) {
            return 1;
        }
    }

    // 北側の塀
    if (CircleIntersectsBox(
        x, z, playerRadius,
        0.0f, 58.5f, 58.5f, 0.13f
    )) {
        return 1;
    }

    // 東側・西側の塀
    if (CircleIntersectsBox(
        x, z, playerRadius,
        -58.5f, 0.0f, 0.13f, 58.5f
    )) {
        return 1;
    }

    if (CircleIntersectsBox(
        x, z, playerRadius,
        58.5f, 0.0f, 0.13f, 58.5f
    )) {
        return 1;
    }

    // 南側の塀：門の左右だけ判定する
    if (CircleIntersectsBox(
        x, z, playerRadius,
        -31.25f, -58.5f, 27.25f, 0.13f
    )) {
        return 1;
    }

    if (CircleIntersectsBox(
        x, z, playerRadius,
        31.25f, -58.5f, 27.25f, 0.13f
    )) {
        return 1;
    }

    // 南側中央の門柱
    if (CircleIntersectsBox(
        x, z, playerRadius,
        -4.0f, -58.5f, 0.22f, 0.22f
    )) {
        return 1;
    }

    if (CircleIntersectsBox(
        x, z, playerRadius,
        4.0f, -58.5f, 0.22f, 0.22f
    )) {
        return 1;
    }

    // すべての障害物に接触していない
    return 0;
}

void DrawEnvironment(unsigned int shaderProgram)
{
    // 木の描画
    for (int i = 0; i < TREE_COUNT; i++) {
        DrawModelAtRotated(
            birchTreeID,
            shaderProgram,
            trees[i].x,
            0.0f,
            trees[i].z,
            trees[i].scale,
            trees[i].rotation
        );
    }

    // 低木の描画
    for (int i = 0; i < BUSH_COUNT; i++) {
        DrawModelAtRotated(
            bushID,
            shaderProgram,
            bushes[i].x,
            0.0f,
            bushes[i].z,
            bushes[i].scale,
            bushes[i].rotation
        );
    }

    // 岩の描画
    for (int i = 0; i < ROCK_COUNT; i++) {
        DrawModelAtRotated(
            rockIDs[rocks[i].modelIndex],
            shaderProgram,
            rocks[i].x,
            0.0f,
            rocks[i].z,
            rocks[i].scale,
            rocks[i].rotation
        );
    }

    // 草の描画
    for (int i = 0; i < GRASS_COUNT; i++) {
        DrawModelAtRotated(
            grassID,
            shaderProgram,
            grasses[i].x,
            0.0f,
            grasses[i].z,
            grasses[i].scale,
            grasses[i].rotation
        );
    }

    // 花の描画
    for (int i = 0; i < FLOWER_COUNT; i++) {
        DrawModelAtRotated(
            flowerIDs[flowers[i].type],
            shaderProgram,
            flowers[i].x,
            0.0f,
            flowers[i].z,
            flowers[i].scale,
            flowers[i].rotation
        );
    }

    // 森の外周の塀と南側の門
    DrawFence(shaderProgram);
}

// 待機画面専用：塀と南側の門だけを描画
void DrawWaitingFence(unsigned int shaderProgram)
{
    DrawFence(shaderProgram);
}


// ワールド座標をマップ座標へ変換
static float EnvironmentMapX(
    float worldX, float mapCenterX,
    float centerX, float scale
)
{
    return mapCenterX + (worldX - centerX) * scale;
}

static float EnvironmentMapY(
    float worldZ, float mapCenterY,
    float centerZ, float scale
)
{
    return mapCenterY - (worldZ - centerZ) * scale;
}

// マップ上に森の配置と塀を描画
void DrawEnvironmentMap(
    float mapCenterX,
    float mapCenterY,
    float centerX,
    float centerZ,
    float scale
)
{
    glLineWidth(2.0f);
    glColor3f(0.62f, 0.39f, 0.20f);

    glBegin(GL_LINES);

    // 北側
    glVertex2f(mapCenterX + (-58.5f - centerX) * scale,
               mapCenterY - (58.5f - centerZ) * scale);
    glVertex2f(mapCenterX + (58.5f - centerX) * scale,
               mapCenterY - (58.5f - centerZ) * scale);

    // 東側
    glVertex2f(mapCenterX + (58.5f - centerX) * scale,
               mapCenterY - (-58.5f - centerZ) * scale);
    glVertex2f(mapCenterX + (58.5f - centerX) * scale,
               mapCenterY - (58.5f - centerZ) * scale);

    // 西側
    glVertex2f(mapCenterX + (-58.5f - centerX) * scale,
               mapCenterY - (-58.5f - centerZ) * scale);
    glVertex2f(mapCenterX + (-58.5f - centerX) * scale,
               mapCenterY - (58.5f - centerZ) * scale);

    // 南側（中央の門を空ける）
    glVertex2f(mapCenterX + (-58.5f - centerX) * scale,
               mapCenterY - (-58.5f - centerZ) * scale);
    glVertex2f(mapCenterX + (-4.0f - centerX) * scale,
               mapCenterY - (-58.5f - centerZ) * scale);

    glVertex2f(mapCenterX + (4.0f - centerX) * scale,
               mapCenterY - (-58.5f - centerZ) * scale);
    glVertex2f(mapCenterX + (58.5f - centerX) * scale,
               mapCenterY - (-58.5f - centerZ) * scale);

    // 開いた門扉
    glVertex2f(mapCenterX + (-4.0f - centerX) * scale,
               mapCenterY - (-58.5f - centerZ) * scale);
    glVertex2f(mapCenterX + (-4.0f - centerX) * scale,
               mapCenterY - (-63.0f - centerZ) * scale);

    glVertex2f(mapCenterX + (4.0f - centerX) * scale,
               mapCenterY - (-58.5f - centerZ) * scale);
    glVertex2f(mapCenterX + (4.0f - centerX) * scale,
               mapCenterY - (-63.0f - centerZ) * scale);

    glEnd();
}
