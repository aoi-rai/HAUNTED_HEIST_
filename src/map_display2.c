#include <GL/glew.h>
#include <math.h>
#include "room2.h"
#include "map_display2.h"

void DrawMap(
    int mapMode,
    float playerX,
    float playerZ,
    float cameraYaw
)
{
    const int screenHeight = 720;

    const float mapLeft = 15.0f;
    const float mapTop = 15.0f;
    const float mapSize = 220.0f;

    float centerX;
    float centerZ;
    float scale;

    if (mapMode == 0) {
        centerX = playerX;
        centerZ = playerZ;
        scale = 7.0f;
    } else {
        centerX = 0.0f;
        centerZ = 0.0f;
        scale = 3.7f;
    }

    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);

    glViewport(0, 0, 1000, 720);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1000, 720, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // マップパネル背景
    glColor4f(0.025f, 0.055f, 0.045f, 0.95f);

    glBegin(GL_QUADS);
    glVertex2f(5, 5);
    glVertex2f(245, 5);
    glVertex2f(245, 245);
    glVertex2f(5, 245);
    glEnd();

    // マップ領域
    glColor3f(0.07f, 0.12f, 0.10f);

    glBegin(GL_QUADS);
    glVertex2f(mapLeft, mapTop);
    glVertex2f(mapLeft + mapSize, mapTop);
    glVertex2f(mapLeft + mapSize, mapTop + mapSize);
    glVertex2f(mapLeft, mapTop + mapSize);
    glEnd();

    // マップのクリッピング
    glEnable(GL_SCISSOR_TEST);

    glScissor(
        15,
        screenHeight - 235,
        220,
        220
    );

    // --------------------------------
    // 部屋を立体ブロック風に描画
    // --------------------------------
    for (int i = 0; i < ROOM_COUNT; i++) {

        Room room = GetRoom(i);

        float x =
            mapLeft + mapSize / 2.0f
            + (room.x - centerX) * scale;

        float y =
            mapTop + mapSize / 2.0f
            - (room.z - centerZ) * scale;

        float w = room.width * scale;
        float h = room.depth * scale;

        // 斜め方向の立体感
        float depth = 5.0f;

        float left = x - w / 2.0f;
        float right = x + w / 2.0f;
        float top = y - h / 2.0f;
        float bottom = y + h / 2.0f;

        // 側面
        glColor3f(0.07f, 0.13f, 0.10f);

        glBegin(GL_QUADS);

        glVertex2f(left, bottom);
        glVertex2f(right, bottom);
        glVertex2f(right + depth, bottom - depth);
        glVertex2f(left + depth, bottom - depth);

        glVertex2f(right, top);
        glVertex2f(right + depth, top - depth);
        glVertex2f(right + depth, bottom - depth);
        glVertex2f(right, bottom);

        glEnd();

        // 部屋の上面
        if (i == 0) {
            glColor3f(0.30f, 0.43f, 0.32f);
        } else {
            glColor3f(0.17f, 0.29f, 0.22f);
        }

        glBegin(GL_QUADS);

        glVertex2f(left, top);
        glVertex2f(right, top);
        glVertex2f(right, bottom);
        glVertex2f(left, bottom);

        glEnd();

        // 枠線
        glColor3f(0.55f, 0.72f, 0.58f);
        glLineWidth(1.5f);

        glBegin(GL_LINE_LOOP);

        glVertex2f(left, top);
        glVertex2f(right, top);
        glVertex2f(right, bottom);
        glVertex2f(left, bottom);

        glEnd();
    }

    // プレイヤーの実際の座標をマップ座標に変換
float playerMapX =
    mapLeft + mapSize / 2.0f
    + (playerX - centerX) * scale;

float playerMapY =
    mapTop + mapSize / 2.0f
    - (playerZ - centerZ) * scale;

    // プレイヤーの向きに合わせたライト表示
float lightLength = 65.0f;
float lightAngle = 0.55f;

float directionX = -sinf(cameraYaw);
float directionY = cosf(cameraYaw);

// 半透明の光
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

glColor4f(0.55f, 1.0f, 0.45f, 0.20f);

glBegin(GL_TRIANGLE_FAN);

glVertex2f(playerMapX, playerMapY);

for (int i = -12; i <= 12; i++) {

    float angle =
        atan2f(directionY, directionX)
        + lightAngle * (float)i / 12.0f;

    float x =
        playerMapX + cosf(angle) * lightLength;

    float y =
        playerMapY + sinf(angle) * lightLength;

    glVertex2f(x, y);
}

glEnd();

glDisable(GL_BLEND);

// プレイヤー本体の丸
glPointSize(10.0f);
glColor3f(0.2f, 1.0f, 0.4f);

glBegin(GL_POINTS);
glVertex2f(playerMapX, playerMapY);
glEnd();

    glDisable(GL_SCISSOR_TEST);

    // マップ枠
    glColor3f(0.45f, 0.65f, 0.50f);
    glLineWidth(2.0f);

    glBegin(GL_LINE_LOOP);

    glVertex2f(5, 5);
    glVertex2f(245, 5);
    glVertex2f(245, 245);
    glVertex2f(5, 245);

    glEnd();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
}

// ========================================
