#include <GL/glew.h>

#include "game_render2.h"
#include "model2.h"
#include "room2.h"
#include "environment2.h"
#include "npc2.h"

#define CAMERA_EDITOR 0

// 待機画面専用：森の環境だけを描画
void DrawWaitingBackground(unsigned int shaderProgram)
{
    glViewport(0, 0, 1000, 720);

    glClear(
        GL_COLOR_BUFFER_BIT |
        GL_DEPTH_BUFFER_BIT
    );

    DrawEnvironment(shaderProgram);
}

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

// 盲目状態の画面暗転
void DrawBlindnessOverlay(float blindnessTimer)
{
    if (blindnessTimer <= 0.0f) {
        return;
    }

    GLint viewport[4];
    GLint previousProgram;
    GLint previousMatrixMode;

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgram);
    glGetIntegerv(GL_MATRIX_MODE, &previousMatrixMode);

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // シェーダーを解除して固定機能描画に切り替える
    glUseProgram(0);

    // 画面全体に描画するための設定
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 画面座標に合わせた投影行列
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, viewport[2], viewport[3], 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // 半透明の黒い画面を重ねる
    glColor4f(0.0f, 0.0f, 0.0f, 0.95f);

    glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f((float)viewport[2], 0.0f);
        glVertex2f((float)viewport[2], (float)viewport[3]);
        glVertex2f(0.0f, (float)viewport[3]);
    glEnd();

    // 行列を元に戻す
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(previousMatrixMode);

    glUseProgram(previousProgram);
    glPopAttrib();
}
