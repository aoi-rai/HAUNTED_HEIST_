#include <stdio.h>

#include "map_update2.h"
#include "room2.h"
#include "map_random2.h"
#include "npc2.h"

// プレイヤーが現在いる部屋を調べる
static int FindPlayerRoom(
    float playerX,
    float playerZ,
    Room rooms[ROOM_COUNT]
)
{
    for (int i = 1; i < ROOM_COUNT; i++) {
        float minX = rooms[i].x - rooms[i].width / 2.0f;
        float maxX = rooms[i].x + rooms[i].width / 2.0f;
        float minZ = rooms[i].z - rooms[i].depth / 2.0f;
        float maxZ = rooms[i].z + rooms[i].depth / 2.0f;

        if (playerX >= minX && playerX <= maxX &&
            playerZ >= minZ && playerZ <= maxZ) {
            return i;
        }
    }

    return 0;
}

// 指定された配置を適用する共通処理
void ApplyMapLayout(
    const float positions[ROOM_COUNT][2],
    Uint32 currentTime,
    Uint32 *lastMapChange,
    float *playerX,
    float *playerZ
)
{
    Room oldRooms[ROOM_COUNT];

    for (int i = 0; i < ROOM_COUNT; i++) {
        oldRooms[i] = GetRoom(i);
    }

    int playerRoomID = FindPlayerRoom(
        *playerX,
        *playerZ,
        oldRooms
    );

    // 部屋の座標を受信データに合わせる
    for (int i = 1; i < ROOM_COUNT; i++) {
        SetRoomPosition(i, positions[i][0], positions[i][1]);
    }

    // 部屋と一緒にNPCを移動
    for (int i = 1; i < ROOM_COUNT; i++) {
        Room newRoom = GetRoom(i);

        float dx = newRoom.x - oldRooms[i].x;
        float dz = newRoom.z - oldRooms[i].z;

        MoveNPCsWithRoom(i, dx, dz);

        printf(
            "部屋%d: X=%.1f Z=%.1f 移動量X=%.1f Z=%.1f\n",
            i, newRoom.x, newRoom.z, dx, dz
        );
    }

    // プレイヤーも所属していた部屋と一緒に移動
    if (playerRoomID > 0) {
        Room newRoom = GetRoom(playerRoomID);

        *playerX += newRoom.x - oldRooms[playerRoomID].x;
        *playerZ += newRoom.z - oldRooms[playerRoomID].z;

        printf(
            "プレイヤーが部屋%dと一緒に移動しました\n",
            playerRoomID
        );
    }

    *lastMapChange = currentTime;
    printf("===== マップ配置の適用完了 =====\n");
}

// 新しい配置を生成する（この時点では現在のマップに適用しない）
void GenerateMapLayout(float positions[ROOM_COUNT][2])
{
    Room newRooms[ROOM_COUNT];

    // 現在の部屋配置を一時配列にコピー
    for (int i = 0; i < ROOM_COUNT; i++) {
        newRooms[i] = GetRoom(i);
    }

    // 一時配列だけをランダム配置する
    RandomizeRoomPositions(newRooms);

    // 新しい座標を取り出す
    for (int i = 0; i < ROOM_COUNT; i++) {
        positions[i][0] = newRooms[i].x;
        positions[i][1] = newRooms[i].z;
    }

    printf("新しいマップ配置を生成しました。\n");
}

// ホスト側：一定時間ごとに新しい配置を決める
void UpdateMap(
    Uint32 currentTime,
    Uint32 *lastMapChange,
    Uint32 mapChangeInterval,
    float *playerX,
    float *playerZ
)
{
    if (currentTime - *lastMapChange < mapChangeInterval) {
        return;
    }

    printf("\n===== 新しいマップ配置を生成します =====\n");

    // 現在の部屋配置を一時配列にコピー
    Room newRooms[ROOM_COUNT];

    for (int i = 0; i < ROOM_COUNT; i++) {
        newRooms[i] = GetRoom(i);
    }

    // 一時配列だけをランダム配置する
    RandomizeRoomPositions(newRooms);

    // 新しい座標を取り出す
    float positions[ROOM_COUNT][2] = {{0}};

    for (int i = 1; i < ROOM_COUNT; i++) {
        positions[i][0] = newRooms[i].x;
        positions[i][1] = newRooms[i].z;
    }

    // 現在の配置と比較しながら新しい配置を適用
    ApplyMapLayout(
        positions,
        currentTime,
        lastMapChange,
        playerX,
        playerZ
    );
}
