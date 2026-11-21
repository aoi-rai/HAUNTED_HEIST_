#include <stdio.h>

#include "map_update2.h"
#include "room2.h"
#include "map_random2.h"
#include "npc2.h"

void UpdateMap(
    Uint32 currentTime,
    Uint32 *lastMapChange,
    Uint32 mapChangeInterval,
    float *playerX,
    float *playerZ
) {
    if (currentTime - *lastMapChange < mapChangeInterval) {
        return;
    }

    printf("\n===== マップを変更します =====\n");

    // 変更前の部屋情報を保存
    Room oldRooms[ROOM_COUNT];

    for (int i = 0; i < ROOM_COUNT; i++) {
        oldRooms[i] = GetRoom(i);
    }

    // 部屋の配置をランダム変更
    RandomizeRooms();

    // 各小部屋の移動量を計算
    for (int i = 1; i < ROOM_COUNT; i++) {
        Room oldRoom = oldRooms[i];
        Room newRoom = GetRoom(i);

        float dx = newRoom.x - oldRoom.x;
        float dz = newRoom.z - oldRoom.z;

        // NPCを移動
        MoveNPCsWithRoom(
            i,
            oldRoom.x,
            oldRoom.z,
            oldRoom.width,
            oldRoom.depth,
            dx,
            dz
        );

        // プレイヤーが変更前の部屋にいるか確認
        float minX = oldRoom.x - oldRoom.width / 2.0f;
        float maxX = oldRoom.x + oldRoom.width / 2.0f;
        float minZ = oldRoom.z - oldRoom.depth / 2.0f;
        float maxZ = oldRoom.z + oldRoom.depth / 2.0f;

        if (*playerX >= minX && *playerX <= maxX &&
            *playerZ >= minZ && *playerZ <= maxZ) {

            *playerX += dx;
            *playerZ += dz;

            printf("プレイヤーが部屋%dと一緒に移動しました\n", i);
        }

        printf(
            "部屋%d: X=%.1f Z=%.1f 移動量X=%.1f Z=%.1f\n",
            i,
            newRoom.x,
            newRoom.z,
            dx,
            dz
        );
    }

    *lastMapChange = currentTime;

    printf("===== マップ変更完了 =====\n");
}
