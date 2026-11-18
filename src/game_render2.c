#include <GL/glew.h>

#include "game_render2.h"
#include "model2.h"
#include "room2.h"
#include "environment2.h"
#include "npc2.h"

#define CAMERA_EDITOR 0

void DrawGameScene(
    int largeRoomID,
    int smallRoomID,
    int playerModelID,
    unsigned int shaderProgram,
    int cameraMode,
    int selectedNPCID,
    float playerX,
    float playerY,
    float playerZ,
    float playerScale,
    float playerDirection
) {
    // 画面全体をクリア
    glViewport(0, 0, 1000, 720);

    glClear(
        GL_COLOR_BUFFER_BIT |
        GL_DEPTH_BUFFER_BIT
    );

    // ゲーム画面のビューポート
    glViewport(0, 0, 1000, 720);

    // 大広間の描画
    DrawModelAt(
        largeRoomID,
        shaderProgram,
        0.0f,
        0.0f,
        0.0f,
        1.98f
    );

    // 小部屋の描画
    for (int i = 1; i < ROOM_COUNT; i++) {
        Room room = GetRoom(i);

        DrawModelAt(
            smallRoomID,
            shaderProgram,
            room.x,
            0.0f,
            room.z,
            1.98f
        );
    }

    // 環境オブジェクトの描画
    DrawEnvironment(shaderProgram);

    // NPCの描画
    DrawNPCs(
        playerModelID,
        shaderProgram,
        selectedNPCID,
        playerScale
    );

    // 制作・確認用カメラのときだけプレイヤーを描画
    if (cameraMode == CAMERA_EDITOR &&
        playerModelID != 0) {

        DrawModelAtRotated(
            playerModelID,
            shaderProgram,
            playerX,
            playerY,
            playerZ,
            playerScale,
            playerDirection
        );
    }
}
