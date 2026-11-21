#ifndef ENVIRONMENT2_H
#define ENVIRONMENT2_H

// 環境モデルの読み込みと配置初期化
int InitEnvironment(void);

// 環境モデルの描画
void DrawEnvironment(unsigned int shaderProgram);

// 待機画面専用：塀と門だけを描画
void DrawWaitingFence(unsigned int shaderProgram);

// 環境モデルの解放
void FreeEnvironment(void);

// プレイヤーと森の障害物・塀の衝突判定
int CheckForestCollision(float x, float z, float playerRadius);


// マップ上に森の配置と塀を描画
void DrawEnvironmentMap(
    float mapCenterX,
    float mapCenterY,
    float centerX,
    float centerZ,
    float scale
);

#endif
