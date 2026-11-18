#include <stdio.h>
#include <GL/glew.h>

#include "room2.h"
#include "furniture2.h"
#include "npc2.h"
#include "environment2.h"
#include "camera2.h"
#include "game_init2.h"
#include "remote_player2.h"

#define PI 3.14159265f

int InitGameWorld(void)
{
    InitRooms();
    InitFurniture();
    InitNPCs();

    if (!InitEnvironment()) {
        printf("環境オブジェクトの初期化失敗\n");
        return 0;
    }

    return 1;
}

void InitGameState(
    GameInitData *game,
    unsigned int shaderProgram
)
{
    // 他プレイヤー情報の初期化
    InitRemotePlayers();
    
    // プレイヤーの初期位置
    game->playerX = 0.0f;
    game->playerY = 0.0f;
    game->playerZ = 0.0f;

    GetPlayerStartPosition(
        &game->playerX,
        &game->playerY,
        &game->playerZ
    );

    game->playerScale = 0.01f;
    game->playerDirection = 1.5707963f;
    game->playerTargetDirection = 1.5707963f;
    game->playerRotationSpeed = PI;

    // 部屋の初期配置を表示
    for (int i = 0; i < ROOM_COUNT; i++) {
        Room room = GetRoom(i);

        printf(
            "部屋%d: X=%.1f Z=%.1f 幅=%.1f 奥行=%.1f\n",
            room.roomID,
            room.x,
            room.z,
            room.width,
            room.depth
        );
    }

    // カメラ設定
    game->cameraMode = CAMERA_PLAYER;
    game->cameraYaw = 0.0f;
    game->cameraPitch = 0.0f;

    // マップ表示モード
    game->mapMode = 0;

    // NPC選択情報
    game->selectedNPCID = -1;
    game->previousSelectedNPCID = -2;

    // 状態異常タイマーの初期化
    game->blindnessTimer = 0.0f;
    game->immobilizedTimer = 0.0f;
    game->invincibleTimer = 0.0f;
    game->reverseControlsTimer = 0.0f;

    // お宝所持状態の初期化
    game->hasTreasure = 0;

    // 通常の方向操作
    for (int i = 0; i < 4; i++) {
        game->movementMap[i] = i;
    }

    // カメラ行列
    Update3DCamera(
        game->view,
        game->cameraMode,
        game->playerX,
        game->playerY,
        game->playerZ,
        game->cameraYaw,
        game->cameraPitch
    );

    // 投影行列
    CreatePerspective(
        game->projection,
        60.0f,
        1000.0f / 720.0f,
        0.1f,
        200.0f
    );

    // シェーダーに初期行列を設定
    glUseProgram(shaderProgram);

    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "view"),
        1,
        GL_FALSE,
        game->view
    );

    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "projection"),
        1,
        GL_FALSE,
        game->projection
    );
}
