#include <GL/glew.h>
#include <math.h>
#include "npc2.h"
#include "camera2.h"

#define PI 3.14159265f

static void Normalize(float v[3])
{
    float length = sqrtf(
        v[0] * v[0] +
        v[1] * v[1] +
        v[2] * v[2]
    );

    if (length > 0.0f) {
        v[0] /= length;
        v[1] /= length;
        v[2] /= length;
    }
}

// ----------------------------------------
// 内積
// ----------------------------------------
static float Dot(const float a[3], const float b[3])
{
    return
        a[0] * b[0] +
        a[1] * b[1] +
        a[2] * b[2];
}

// ----------------------------------------
// 外積
// ----------------------------------------
static void Cross(
    const float a[3],
    const float b[3],
    float result[3]
)
{
    result[0] = a[1] * b[2] - a[2] * b[1];
    result[1] = a[2] * b[0] - a[0] * b[2];
    result[2] = a[0] * b[1] - a[1] * b[0];
}

// ----------------------------------------
// 透視投影行列
// ----------------------------------------
void CreatePerspective(
    float matrix[16],
    float fov,
    float aspect,
    float nearPlane,
    float farPlane
)
{
    float f = 1.0f / tanf(fov * PI / 360.0f);

    for (int i = 0; i < 16; i++) {
        matrix[i] = 0.0f;
    }

    matrix[0] = f / aspect;
    matrix[5] = f;

    matrix[10] =
        (farPlane + nearPlane) /
        (nearPlane - farPlane);

    matrix[11] = -1.0f;

    matrix[14] =
        (2.0f * farPlane * nearPlane) /
        (nearPlane - farPlane);
}

// ----------------------------------------
// カメラ行列
// ----------------------------------------
static void CreateLookAt(
    float matrix[16],
    const float eye[3],
    const float center[3],
    const float up[3]
)
{
    float forward[3] = {
        center[0] - eye[0],
        center[1] - eye[1],
        center[2] - eye[2]
    };

    Normalize(forward);

    float side[3];
    Cross(forward, up, side);
    Normalize(side);

    float cameraUp[3];
    Cross(side, forward, cameraUp);

    matrix[0] = side[0];
    matrix[1] = cameraUp[0];
    matrix[2] = -forward[0];
    matrix[3] = 0.0f;

    matrix[4] = side[1];
    matrix[5] = cameraUp[1];
    matrix[6] = -forward[1];
    matrix[7] = 0.0f;

    matrix[8] = side[2];
    matrix[9] = cameraUp[2];
    matrix[10] = -forward[2];
    matrix[11] = 0.0f;

    matrix[12] = -Dot(side, eye);
    matrix[13] = -Dot(cameraUp, eye);
    matrix[14] = Dot(forward, eye);
    matrix[15] = 1.0f;
}

// ----------------------------------------
// カメラの視点を更新
// ----------------------------------------
void Update3DCamera(
    float view[16],
    int cameraMode,
    float playerX,
    float playerY,
    float playerZ,
    float cameraYaw,
    float cameraPitch
)
{
    float eye[3];
    float center[3];
    float up[3] = {0.0f, 1.0f, 0.0f};

    if (cameraMode == CAMERA_EDITOR) {

        // 制作・確認用カメラ
        eye[0] = 25.0f;
        eye[1] = 32.0f;
        eye[2] = 38.0f;

        center[0] = 0.0f;
        center[1] = 0.0f;
        center[2] = 0.0f;

    } else {

        // 一人称カメラの位置
        eye[0] = playerX;
        eye[1] = playerY + 2.0f;
        eye[2] = playerZ;

        // カメラの向いている方向
        float directionX =
            -sinf(cameraYaw) * cosf(cameraPitch);

        float directionY =
            sinf(cameraPitch);

        float directionZ =
            -cosf(cameraYaw) * cosf(cameraPitch);

        // 視線の先
        center[0] = eye[0] + directionX;
        center[1] = eye[1] + directionY;
        center[2] = eye[2] + directionZ;
    }

    CreateLookAt(view, eye, center, up);
}

// ----------------------------------------
// 画面中央の視線上にいるNPCを探す
// ----------------------------------------
// ----------------------------------------
// 画面中央の照準を描画
// ----------------------------------------
void DrawReticle(int selected)
{
    // シェーダーを解除して2D描画
    glUseProgram(0);

    // 深度判定を一時的に無効化
    glDisable(GL_DEPTH_TEST);

    // 画面全体を2D座標にする
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    if (selected) {
        // NPC選択中：緑色
        glColor3f(0.3f, 1.0f, 0.4f);
    } else {
        // 通常時：白色
        glColor3f(1.0f, 1.0f, 1.0f);
    }

    glLineWidth(2.0f);

    glBegin(GL_LINES);

    if (selected) {

        // 中央の円形照準
        const float radius = 0.055f;

        for (int i = 0; i < 32; i++) {
            float a1 = 2.0f * PI * i / 32.0f;
            float a2 = 2.0f * PI * (i + 1) / 32.0f;

            glVertex2f(
                cosf(a1) * radius,
                sinf(a1) * radius
            );

            glVertex2f(
                cosf(a2) * radius,
                sinf(a2) * radius
            );
        }

        // 中央の小さな十字
        glVertex2f(-0.018f, 0.0f);
        glVertex2f( 0.018f, 0.0f);

        glVertex2f(0.0f, -0.018f);
        glVertex2f(0.0f,  0.018f);

    } else {

        // 通常時の十字照準
        glVertex2f(-0.035f, 0.0f);
        glVertex2f(-0.012f, 0.0f);

        glVertex2f(0.012f, 0.0f);
        glVertex2f(0.035f, 0.0f);

        glVertex2f(0.0f, -0.035f);
        glVertex2f(0.0f, -0.012f);

        glVertex2f(0.0f, 0.012f);
        glVertex2f(0.0f, 0.035f);
    }

    glEnd();

    // 行列を元に戻す
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);

    // OpenGLの状態を戻す
    glEnable(GL_DEPTH_TEST);
}

// ----------------------------------------
