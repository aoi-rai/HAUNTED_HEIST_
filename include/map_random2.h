#ifndef MAP_RANDOM_H
#define MAP_RANDOM_H

#include "room2.h"

// 小部屋の配置をランダムに決定
void RandomizeRoomPositions(Room rooms[ROOM_COUNT]);

// 部屋の内装をランダムに決定
void RandomizeRoomInterior(int roomID);

// 内装の変更
void ChangeRoomInterior(int roomID, int interiorID);

#endif
