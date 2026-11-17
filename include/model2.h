#ifndef MODEL_H
#define MODEL_H

// 3Dモデルを読み込む
int LoadModel(const char *filename);

// 通常の描画
void DrawModelAt(
    int modelID,
    unsigned int shaderProgram,
    float x,
    float y,
    float z,
    float scale
);

// Y軸回転に対応した描画
void DrawModelAtRotated(
    int modelID,
    unsigned int shaderProgram,
    float x,
    float y,
    float z,
    float scale,
    float rotationY
);

void DrawModelAtRotatedOutline(
    int modelID,
    unsigned int shaderProgram,
    float x,
    float y,
    float z,
    float scale,
    float rotationY
);

// モデルの解放
void FreeModel(int modelID);

#endif
