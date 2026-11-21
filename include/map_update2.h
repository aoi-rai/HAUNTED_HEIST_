#ifndef MAP_UPDATE2_H
#define MAP_UPDATE2_H

#include <SDL2/SDL.h>
#include "room2.h"

// 新しい部屋配置を生成する
void GenerateMapLayout(float positions[ROOM_COUNT][2]);

// ホスト側：ランダムな配置を決定して適用
void UpdateMap(
    Uint32 currentTime,
    Uint32 *lastMapChange,
    Uint32 mapChangeInterval,
    float *playerX,
    float *playerZ
);

// 全クライアント：受信した配置を適用
void ApplyMapLayout(
    const float positions[ROOM_COUNT][2],
    Uint32 currentTime,
    Uint32 *lastMapChange,
    float *playerX,
    float *playerZ
);

#endif
