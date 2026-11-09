#ifndef MODEL_H
#define MODEL_H

// 3Dモデルの読み込み
int LoadModel(const char *filename);

// 3Dモデルの描画
void DrawModel(int modelID);

// 3Dモデルの解放
void FreeModel(int modelID);

#endif
