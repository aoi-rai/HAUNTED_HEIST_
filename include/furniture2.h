#ifndef FURNITURE2_H
#define FURNITURE2_H

#define MAX_FURNITURE 100

typedef struct {
    int furnitureID;
    int roomID;

    // 部屋の中心からの相対座標
    float localX;
    float localZ;

    // 高さ・向き・大きさ
    float y;
    float rotationY;
    float scale;
} Furniture;

// 家具の初期化
void InitFurniture(void);

// 家具を追加
int AddFurniture(
    int roomID,
    float localX,
    float localZ,
    float y,
    float rotationY,
    float scale
);

// 家具の数を取得
int GetFurnitureCount(void);

// 家具の情報を取得
Furniture GetFurniture(int furnitureID);

// 家具のワールド座標を取得
void GetFurnitureWorldPosition(
    int furnitureID,
    float *x,
    float *y,
    float *z
);

#endif
