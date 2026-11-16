#ifndef MODEL_H
#define MODEL_H

// 3Dモデルの読み込み
int LoadModel(const char *filename);

// 位置と大きさを指定して描画
void DrawModelAt(
    int modelID,
    unsigned int shaderProgram,
    float x,
    float y,
    float z,
    float scale
);

// 3Dモデルの解放
void FreeModel(int modelID);

#endif
