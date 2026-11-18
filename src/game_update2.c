#include "game_update2.h"

#include "npc2.h"
#include "input2.h"
#include "player2.h"
#include "map_update2.h"
#include "camera2.h"
#include "target2.h"

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

    // プレイヤーの移動・旋回
    UpdatePlayerMovement(
        keys,
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
