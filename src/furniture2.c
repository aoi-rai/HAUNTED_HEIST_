#include <stddef.h>

#include "furniture2.h"
#include "room2.h"

// 家具の情報を保存する配列
static Furniture furnitureList[MAX_FURNITURE];

// 現在登録されている家具の数
static int furnitureCount = 0;


// 家具の初期化
void InitFurniture(void)
{
    furnitureCount = 0;
}


// 家具を追加
int AddFurniture(
    int roomID,
    float localX,
    float localZ,
    float y,
    float rotationY,
    float scale
)
{
    // 登録数の上限を確認
    if (furnitureCount >= MAX_FURNITURE) {
        return -1;
    }

    // 部屋番号を確認
    if (roomID < 0 || roomID >= ROOM_COUNT) {
        return -1;
    }

    int id = furnitureCount;

    furnitureList[id] = (Furniture){
        id,
        roomID,
        localX,
        localZ,
        y,
        rotationY,
        scale
    };

    furnitureCount++;

    return id;
}


// 家具の数を取得
int GetFurnitureCount(void)
{
    return furnitureCount;
}


// 家具の情報を取得
Furniture GetFurniture(int furnitureID)
{
    if (furnitureID < 0 ||
        furnitureID >= furnitureCount) {
        return (Furniture){-1, -1, 0, 0, 0, 0, 0};
    }

    return furnitureList[furnitureID];
}


// 家具のワールド座標を取得
void GetFurnitureWorldPosition(
    int furnitureID,
    float *x,
    float *y,
    float *z
)
{
    if (x == NULL || y == NULL || z == NULL) {
        return;
    }

    Furniture furniture = GetFurniture(furnitureID);

    if (furniture.furnitureID == -1) {
        *x = 0.0f;
        *y = 0.0f;
        *z = 0.0f;
        return;
    }

    // 家具が所属する部屋の情報を取得
    Room room = GetRoom(furniture.roomID);

    // 部屋の座標 + 家具の相対座標
    *x = room.x + furniture.localX;
    *y = furniture.y;
    *z = room.z + furniture.localZ;
}
