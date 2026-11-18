#ifndef ENVIRONMENT2_H
#define ENVIRONMENT2_H

// 環境モデルの読み込みと配置初期化
int InitEnvironment(void);

// 環境モデルの描画
void DrawEnvironment(unsigned int shaderProgram);

// 環境モデルの解放
void FreeEnvironment(void);

#endif
