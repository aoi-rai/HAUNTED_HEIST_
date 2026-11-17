#include <stdio.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <SDL2/SDL_image.h>

#include "model2.h"
#include "room2.h"
#include "shader2.h"
#include "furniture2.h"
#include "npc2.h"
#include "map_random2.h"

// カメラの種類
#define CAMERA_EDITOR 0
#define CAMERA_PLAYER 1

#define PI 3.14159265f

// ----------------------------------------
// ベクトルの正規化
// ----------------------------------------
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
static void CreatePerspective(
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
static void UpdateCamera(
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
static int FindTargetNPC(
    int cameraMode,
    float playerX,
    float playerY,
    float playerZ,
    float cameraYaw,
    float cameraPitch
)
{
    if (cameraMode != CAMERA_PLAYER) {
        return -1;
    }

    // 一人称カメラの位置
    float eye[3] = {
        playerX,
        playerY + 2.0f,
        playerZ
    };

    // カメラの視線方向
    float direction[3] = {
        -sinf(cameraYaw) * cosf(cameraPitch),
        sinf(cameraPitch),
        -cosf(cameraYaw) * cosf(cameraPitch)
    };

    Normalize(direction);

    int selectedID = -1;

    // 最も近いNPCまでの距離
    float nearestDistance = 5.0f;

    for (int i = 0; i < NPC_COUNT; i++) {

        NPC npc = GetNPC(i);

        // NPCの中心位置
        float npcCenter[3] = {
            npc.x,
            npc.y + 1.0f,
            npc.z
        };

        // カメラからNPCへの方向
        float toNPC[3] = {
            npcCenter[0] - eye[0],
            npcCenter[1] - eye[1],
            npcCenter[2] - eye[2]
        };

        // 視線方向への距離
        float distance = Dot(toNPC, direction);

        // カメラの後ろにいるNPCは対象外
        if (distance <= 0.0f ||
            distance >= nearestDistance) {
            continue;
        }

        // 視線上の最近接点
        float closestPoint[3] = {
            eye[0] + direction[0] * distance,
            eye[1] + direction[1] * distance,
            eye[2] + direction[2] * distance
        };

        // NPC中心と視線の距離
        float dx = npcCenter[0] - closestPoint[0];
        float dy = npcCenter[1] - closestPoint[1];
        float dz = npcCenter[2] - closestPoint[2];

        float distanceSquared =
            dx * dx + dy * dy + dz * dz;

        // NPCの選択判定
        if (distanceSquared <= 1.0f) {
            nearestDistance = distance;
            selectedID = i;
        }
    }

    return selectedID;
}

// ----------------------------------------
// 画面中央の照準を描画
// ----------------------------------------
static void DrawReticle(int selected)
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
// マップ描画
// ----------------------------------------
static void DrawMap(
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
// メイン関数
// ========================================
int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    // 乱数の初期化
    srand((unsigned int)time(NULL));

    // ------------------------------------
    // SDLの初期化
    // ------------------------------------
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL初期化失敗: %s\n", SDL_GetError());
        return 1;
    }

    // PNG画像読み込みの初期化
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
        printf("SDL_image初期化失敗: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    printf("PNG画像読み込みの初期化成功\n");

    // ------------------------------------
    // OpenGLの設定
    // ------------------------------------
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    // ウィンドウの作成
    SDL_Window *window = SDL_CreateWindow(
        "HAUNTED HEIST",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1000,
        720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    if (window == NULL) {
        printf("ウィンドウ作成失敗: %s\n", SDL_GetError());

        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // OpenGLコンテキスト作成
    SDL_GLContext context = SDL_GL_CreateContext(window);

    if (context == NULL) {
        printf("OpenGL初期化失敗: %s\n", SDL_GetError());

        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // GLEWの初期化
    GLenum glewResult = glewInit();

    if (glewResult != GLEW_OK) {
        printf(
            "GLEW初期化失敗: %s\n",
            glewGetErrorString(glewResult)
        );

        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    printf("GLEWの初期化成功\n");

    SDL_GL_SetSwapInterval(1);

    glViewport(0, 0, 1000, 720);

    glClearColor(0.05f, 0.10f, 0.08f, 1.0f);

    glEnable(GL_DEPTH_TEST);

    printf("HAUNTED HEIST 起動\n");

    // ------------------------------------
    // シェーダーの読み込み
    // ------------------------------------
    unsigned int shaderProgram = LoadShaders(
        "shaders/vertex.glsl",
        "shaders/fragment.glsl"
    );

    if (shaderProgram == 0) {
        printf("シェーダー読み込み失敗\n");

        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    printf("シェーダーの読み込み成功\n");

    // ------------------------------------
    // 部屋・家具・NPCの初期化
    // ------------------------------------
    InitRooms();
    InitFurniture();
    InitNPCs();

    // ------------------------------------
    // マップ変更タイマー
    // ------------------------------------
    Uint32 lastMapChange = SDL_GetTicks();

    const Uint32 MAP_CHANGE_INTERVAL = 30000;

    // ------------------------------------
    // モデルの読み込み
    // ------------------------------------
    int largeRoomID =
        LoadModel("models/mansion/oohiroma.fbx");

    int smallRoomID =
        LoadModel("models/mansion/kobeya.fbx");

    int playerModelID =
        LoadModel("models/ghost.fbx");

    if (largeRoomID == 0 || smallRoomID == 0) {
        printf("モデル読み込み失敗\n");

        FreeModel(largeRoomID);
        FreeModel(smallRoomID);
        FreeModel(playerModelID);
        FreeShaders(shaderProgram);

        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // ------------------------------------
    // プレイヤーの初期位置
    // ------------------------------------
    float playerX = 0.0f;
    float playerY = 0.0f;
    float playerZ = 0.0f;

    GetPlayerStartPosition(
        &playerX,
        &playerY,
        &playerZ
    );

    // プレイヤーモデルの大きさ
    float playerScale = 0.01f;

    // プレイヤーの向き
    float playerDirection = 1.5707963f;
    float playerTargetDirection = 1.5707963f;

    // 旋回速度
    const float playerRotationSpeed = PI;

    // ------------------------------------
    // 部屋の情報を確認
    // ------------------------------------
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

    // ------------------------------------
    // カメラ設定
    // ------------------------------------
    float view[16];
    float projection[16];

    int cameraMode = CAMERA_PLAYER;

// マップ表示モード
// 0 = 周辺マップ
// 1 = 全体マップ
int mapMode = 0;

    float cameraYaw = 0.0f;
    float cameraPitch = 0.0f;

    // NPC選択情報
    int selectedNPCID = -1;
    int previousSelectedNPCID = -2;

    // 初期カメラ行列
    UpdateCamera(
        view,
        cameraMode,
        playerX,
        playerY,
        playerZ,
        cameraYaw,
        cameraPitch
    );

    // ------------------------------------
    // 透視投影行列
    // ------------------------------------
    CreatePerspective(
        projection,
        60.0f,
        1000.0f / 720.0f,
        0.1f,
        200.0f
    );

    // ------------------------------------
    // シェーダー設定
    // ------------------------------------
    glUseProgram(shaderProgram);

    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "view"),
        1,
        GL_FALSE,
        view
    );

    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "projection"),
        1,
        GL_FALSE,
        projection
    );

    // ------------------------------------
    // メインループ
    // ------------------------------------
    int running = 1;

    SDL_Event event;

    Uint32 previousTime = SDL_GetTicks();

    while (running) {

        // ====================================
        // イベント処理
        // ====================================
        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_QUIT) {
                running = 0;
            }

            /// Xキーでマップ切り替え
if (event.type == SDL_KEYDOWN &&
    event.key.repeat == 0 &&
    event.key.keysym.sym == SDLK_x) {

    mapMode = 1 - mapMode;

    if (mapMode == 0) {
        printf("周辺マップに切り替え\n");
    } else {
        printf("全体マップに切り替え\n");
    }
}
        }

        // ====================================
        // 経過時間
        // ====================================
        Uint32 currentTime = SDL_GetTicks();

        float deltaTime =
            (currentTime - previousTime) / 1000.0f;

        previousTime = currentTime;

        // ====================================
        // NPCの移動更新
        // ====================================
        UpdateNPCs(deltaTime);

        // キーボード入力
        const Uint8 *keys = SDL_GetKeyboardState(NULL);

        // ====================================
        // WASDでカメラ操作
        // ====================================
        if (cameraMode == CAMERA_PLAYER) {

            float cameraRotateSpeed = 2.0f * deltaTime;

            // 左右
            if (keys[SDL_SCANCODE_D]) {
                cameraYaw -= cameraRotateSpeed;
            }

            if (keys[SDL_SCANCODE_A]) {
                cameraYaw += cameraRotateSpeed;
            }

            // 上下
            if (keys[SDL_SCANCODE_W]) {
                cameraPitch += cameraRotateSpeed;
            }

            if (keys[SDL_SCANCODE_S]) {
                cameraPitch -= cameraRotateSpeed;
            }

            // 上下の回転範囲
            if (cameraPitch > 1.2f) {
                cameraPitch = 1.2f;
            }

            if (cameraPitch < -1.2f) {
                cameraPitch = -1.2f;
            }
        }

        // カメラの向きにプレイヤーの向きを合わせる
playerTargetDirection = atan2f(
    -cosf(cameraYaw),
    -sinf(cameraYaw)
);

        // ====================================
        // 方向キーでプレイヤー移動
        // ====================================
        float moveX = 0.0f;
        float moveZ = 0.0f;

        // カメラの向いている方向
        float forwardX = -sinf(cameraYaw);
        float forwardZ = -cosf(cameraYaw);

        // カメラから見た右方向
        float rightX = cosf(cameraYaw);
        float rightZ = -sinf(cameraYaw);

        if (cameraMode == CAMERA_PLAYER) {

            // 前進
            if (keys[SDL_SCANCODE_UP]) {
                moveX += forwardX;
                moveZ += forwardZ;
            }

            // 後退
            if (keys[SDL_SCANCODE_DOWN]) {
                moveX -= forwardX;
                moveZ -= forwardZ;
            }

            // 左移動
            if (keys[SDL_SCANCODE_LEFT]) {
                moveX -= rightX;
                moveZ -= rightZ;
            }

            // 右移動
            if (keys[SDL_SCANCODE_RIGHT]) {
                moveX += rightX;
                moveZ += rightZ;
            }
        }

        // 移動速度
        float moveSpeed = 5.0f;

        // 斜め移動の速度調整
        float moveLength = sqrtf(
            moveX * moveX +
            moveZ * moveZ
        );

        if (moveLength > 0.0f) {

            playerX +=
                (moveX / moveLength) *
                moveSpeed *
                deltaTime;

            playerZ +=
                (moveZ / moveLength) *
                moveSpeed *
                deltaTime;

            // 移動方向を目標角度にする
            playerTargetDirection = atan2f(
                -moveZ,
                moveX
            );
        }

        // ====================================
        // プレイヤーの滑らかな旋回
        // ====================================
        {
            const float TWO_PI = 6.28318530f;

            float angleDifference =
                playerTargetDirection - playerDirection;

            while (angleDifference > PI) {
                angleDifference -= TWO_PI;
            }

            while (angleDifference < -PI) {
                angleDifference += TWO_PI;
            }

            float maxTurn =
                playerRotationSpeed * deltaTime;

            if (fabsf(angleDifference) <= maxTurn) {

                playerDirection = playerTargetDirection;

            } else if (angleDifference > 0.0f) {

                playerDirection += maxTurn;

            } else {

                playerDirection -= maxTurn;
            }

            // 角度を -PI ～ PI に収める
            while (playerDirection > PI) {
                playerDirection -= TWO_PI;
            }

            while (playerDirection < -PI) {
                playerDirection += TWO_PI;
            }
        }

        // ====================================
// 30秒ごとのマップ変更
// ====================================
if (currentTime - lastMapChange >= MAP_CHANGE_INTERVAL) {

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

        if (playerX >= minX && playerX <= maxX &&
            playerZ >= minZ && playerZ <= maxZ) {

            playerX += dx;
            playerZ += dz;

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

    lastMapChange = currentTime;

    printf("===== マップ変更完了 =====\n");
}

        // ====================================
        // カメラの更新
        // ====================================
        UpdateCamera(
            view,
            cameraMode,
            playerX,
            playerY,
            playerZ,
            cameraYaw,
            cameraPitch
        );

        // ====================================
        // NPCの選択対象を更新
        // ====================================
        selectedNPCID = FindTargetNPC(
            cameraMode,
            playerX,
            playerY,
            playerZ,
            cameraYaw,
            cameraPitch
        );

        // 選択対象が変わったときだけ表示
        if (selectedNPCID != previousSelectedNPCID) {

            if (selectedNPCID >= 0) {

                printf(
                    "選択中のNPC: %d\n",
                    selectedNPCID
                );

            } else {

                if (cameraMode == CAMERA_PLAYER) {
                    printf("NPCの選択解除\n");
                }
            }

            previousSelectedNPCID = selectedNPCID;
        }

        // ====================================
        // シェーダー設定
        // ====================================
        glUseProgram(shaderProgram);

        glUniformMatrix4fv(
            glGetUniformLocation(shaderProgram, "view"),
            1,
            GL_FALSE,
            view
        );

        glUniformMatrix4fv(
            glGetUniformLocation(shaderProgram, "projection"),
            1,
            GL_FALSE,
            projection
        );

        // ====================================
// 画面全体をクリア
// ====================================
glViewport(0, 0, 1000, 720);

glClear(
    GL_COLOR_BUFFER_BIT |
    GL_DEPTH_BUFFER_BIT
);

// ====================================
// 右側をゲーム画面にする
// ====================================
glViewport(0, 0, 1000, 720);

        // ====================================
        // 大広間の描画
        // ====================================
        DrawModelAt(
            largeRoomID,
            shaderProgram,
            0.0f,
            0.0f,
            0.0f,
            1.98f
        );

        // ====================================
        // 小部屋の描画
        // ====================================
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

// ====================================
// NPCの描画
// ====================================
if (playerModelID != 0) {

    for (int i = 0; i < NPC_COUNT; i++) {

        NPC npc = GetNPC(i);

        // 選択中のNPCだけ輪郭を描画
        if (i == selectedNPCID) {

            DrawModelAtRotatedOutline(
                playerModelID,
                shaderProgram,
                npc.x,
                npc.y,
                npc.z,
                playerScale,
                npc.direction
            );
        }

        // 通常のNPCモデルを描画
        DrawModelAtRotated(
            playerModelID,
            shaderProgram,
            npc.x,
            npc.y,
            npc.z,
            playerScale,
            npc.direction
        );
    }
}

        // ====================================
        // プレイヤーモデルの描画
        // ====================================
        // 制作・確認用カメラのときだけ描画
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

        // 一人称カメラのとき照準を表示
if (cameraMode == CAMERA_PLAYER) {
    DrawReticle(selectedNPCID >= 0);
}

// ====================================
// 左側のマップを描画
// ====================================
DrawMap(
    mapMode,
    playerX,
    playerZ,
    cameraYaw
);

        // ====================================
        // 画面更新
        // ====================================
        SDL_GL_SwapWindow(window);
    }

    // ========================================
    // 終了処理
    // ========================================

    FreeModel(largeRoomID);
    FreeModel(smallRoomID);
    FreeModel(playerModelID);

    FreeShaders(shaderProgram);

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();

    return 0;
}