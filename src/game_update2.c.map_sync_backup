#include "game_update2.h"

#include "npc2.h"
#include "input2.h"
#include "player2.h"
#include "map_update2.h"
#include "camera2.h"
#include "target2.h"
#include "room2.h"

#include <string.h>
#include <stdlib.h>

// プレイヤーを別の小部屋へ移動させる
static void TeleportPlayerToAnotherRoom(GameInitData *game)
{
    int currentRoomID = -1;

    // 現在プレイヤーがいる小部屋を調べる
    for (int i = 1; i < ROOM_COUNT; i++) {
        Room room = GetRoom(i);

        float minX = room.x - room.width / 2.0f;
        float maxX = room.x + room.width / 2.0f;
        float minZ = room.z - room.depth / 2.0f;
        float maxZ = room.z + room.depth / 2.0f;

        if (game->playerX >= minX && game->playerX <= maxX &&
            game->playerZ >= minZ && game->playerZ <= maxZ) {
            currentRoomID = i;
            break;
        }
    }

    // 現在とは別の小部屋を選ぶ
    int targetRoomID;

    do {
        targetRoomID = 1 + rand() % (ROOM_COUNT - 1);
    } while (targetRoomID == currentRoomID);

    Room targetRoom = GetRoom(targetRoomID);

    // 選んだ部屋の中央へ移動
    game->playerX = targetRoom.x;
    game->playerZ = targetRoom.z;

    printf("発覚ペナルティ：部屋%dへワープしました\n", targetRoomID);
}

// プレイヤーが発覚したとき
void ApplyDiscoveredEffect(GameInitData *game)
{
    game->blindnessTimer = 30.0f;
    game->immobilizedTimer = 10.0f;
    game->invincibleTimer = 5.0f;

    // 別の小部屋へワープ
    TeleportPlayerToAnotherRoom(game);
}

// 4方向の操作をランダムにシャッフル
static void RandomizeMovementControls(GameInitData *game)
{
    int newMap[4] = {0, 1, 2, 3};

    // Fisher-Yatesシャッフル
    for (int i = 3; i > 0; i--) {
        int j = rand() % (i + 1);

        int temp = newMap[i];
        newMap[i] = newMap[j];
        newMap[j] = temp;
    }

    // 前回と同じ組み合わせなら再抽選
    while (memcmp(newMap, game->movementMap, sizeof(newMap)) == 0) {
        for (int i = 3; i > 0; i--) {
            int j = rand() % (i + 1);

            int temp = newMap[i];
            newMap[i] = newMap[j];
            newMap[j] = temp;
        }
    }

    memcpy(game->movementMap, newMap, sizeof(newMap));

    printf(
        "操作がランダムに変更されました！ "
        "上=%d 下=%d 左=%d 右=%d\n",
        newMap[0], newMap[1], newMap[2], newMap[3]
    );
}

// NPC状態になったとき
void ApplyNpcStateEffect(GameInitData *game)
{
    game->blindnessTimer = 20.0f;
    game->reverseControlsTimer = 40.0f;
    RandomizeMovementControls(game);
}

// 偽物の宝物を取ったとき
void ApplyFakeTreasureEffect(GameInitData *game)
{
    game->blindnessTimer = 20.0f;
    game->reverseControlsTimer = 40.0f;
    RandomizeMovementControls(game);
}

// 選択中のNPCにアクションする
void HandleNPCAction(GameInitData *game)
{
    printf("[DEBUG] NPCアクション呼び出し: selectedNPCID=%d, hasTreasure=%d\n",
           game->selectedNPCID, game->hasTreasure);

    if (game->selectedNPCID < 0 || game->hasTreasure) {
        printf("[DEBUG] アクション中止: NPC未選択またはお宝所持中\n");
        return;
    }

    NPC npc = GetNPC(game->selectedNPCID);

    if (npc.type == GHOST_REAL_TREASURE) {
        if (TryTakeNPC(game->selectedNPCID)) {
            game->hasTreasure = 1;
            printf("お宝を手に入れた！\\n");
        }
    }
    else if (npc.type == GHOST_FAKE_TREASURE) {
        ApplyFakeTreasureEffect(game);
        printf("ダミーだった！\\n");
    }
    else {
        ApplyNpcStateEffect(game);
        printf("普通のおばけだった！\\n");
    }
}

void UpdateGameFrame(
    GameInitData *game,
    const Uint8 *keys,
    float deltaTime,
    Uint32 currentTime,
    Uint32 *lastMapChange,
    Uint32 mapChangeInterval
)
{
    // NPCの移動更新
    UpdateNPCs(deltaTime);

    // カメラ操作
    UpdateCameraInput(
        keys,
        game->cameraMode,
        deltaTime,
        &game->cameraYaw,
        &game->cameraPitch
    );

    // ====================================
    // 状態異常タイマーの更新
    // ====================================

    if (game->blindnessTimer > 0.0f) {
        game->blindnessTimer -= deltaTime;

        if (game->blindnessTimer < 0.0f) {
            game->blindnessTimer = 0.0f;
        }
    }

    if (game->immobilizedTimer > 0.0f) {
        game->immobilizedTimer -= deltaTime;

        if (game->immobilizedTimer < 0.0f) {
            game->immobilizedTimer = 0.0f;
        }
    }

    if (game->invincibleTimer > 0.0f) {
        game->invincibleTimer -= deltaTime;

        if (game->invincibleTimer < 0.0f) {
            game->invincibleTimer = 0.0f;
        }
    }

    if (game->reverseControlsTimer > 0.0f) {
        game->reverseControlsTimer -= deltaTime;

        if (game->reverseControlsTimer < 0.0f) {
            game->reverseControlsTimer = 0.0f;
        }
    }

    // ====================================
    // プレイヤー移動用のキーを作成
    // ====================================

    Uint8 movementKeys[SDL_NUM_SCANCODES];

    memcpy(
        movementKeys,
        keys,
        sizeof(movementKeys)
    );

    // 行動不能中は移動キーを無効化
    if (game->immobilizedTimer > 0.0f) {
        memset(
            movementKeys,
            0,
            sizeof(movementKeys)
        );
    }
    // 操作反転中は4方向の割り当てをランダムに変更
    else if (game->reverseControlsTimer > 0.0f) {
        const SDL_Scancode directions[4] = {
            SDL_SCANCODE_UP,
            SDL_SCANCODE_DOWN,
            SDL_SCANCODE_LEFT,
            SDL_SCANCODE_RIGHT
        };

        Uint8 originalKeys[4];

        for (int i = 0; i < 4; i++) {
            originalKeys[i] = keys[directions[i]];
            movementKeys[directions[i]] = 0;
        }

        for (int i = 0; i < 4; i++) {
            if (originalKeys[i]) {
                int destination = game->movementMap[i];
                movementKeys[directions[destination]] = 1;
            }
        }
    }

    // プレイヤーの移動・旋回
    UpdatePlayerMovement(
        movementKeys,
        game->cameraMode,
        game->cameraYaw,
        deltaTime,
        game->playerRotationSpeed,
        &game->playerX,
        &game->playerZ,
        &game->playerDirection,
        &game->playerTargetDirection
    );

    // マップ変更
    UpdateMap(
        currentTime,
        lastMapChange,
        mapChangeInterval,
        &game->playerX,
        &game->playerZ
    );

    // カメラ更新
    Update3DCamera(
        game->view,
        game->cameraMode,
        game->playerX,
        game->playerY,
        game->playerZ,
        game->cameraYaw,
        game->cameraPitch
    );

    // NPCの選択対象更新
    UpdateTargetNPC(
        game->cameraMode,
        game->playerX,
        game->playerY,
        game->playerZ,
        game->cameraYaw,
        game->cameraPitch,
        &game->selectedNPCID,
        &game->previousSelectedNPCID
    );
}
