#ifndef COLLISION_H
#define COLLISION_H

// 壁・家具などの当たり判定情報を初期化
void InitMapCollision(void);

// マップ側の当たり判定情報を解放
void FinalizeMapCollision(void);

#endif
