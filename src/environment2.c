#include <stdio.h>
#include <stdlib.h>

#include "model2.h"
#include "environment2.h"

#define TREE_COUNT 120
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

// 部分的に読み込みに失敗した場合も解放できるようにする
void FreeEnvironment(void)
{
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

    // 草の配置情報を初期化（400か所）
    for (int i = 0; i < GRASS_COUNT; i++) {
        float t = (float)rand() / (float)RAND_MAX;
        float offset = -39.0f + t * 78.0f;
        float side = (float)rand() / (float)RAND_MAX;

        if (side < 0.25f) {
            grasses[i].x = -39.0f;
            grasses[i].z = offset;
        }
        else if (side < 0.5f) {
            grasses[i].x = 39.0f;
            grasses[i].z = offset;
        }
        else if (side < 0.75f) {
            grasses[i].x = offset;
            grasses[i].z = -39.0f;
        }
        else {
            grasses[i].x = offset;
            grasses[i].z = 39.0f;
        }

        grasses[i].scale =
            0.022f + ((float)rand() / (float)RAND_MAX) * 0.008f;

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

    return 1;
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
}
