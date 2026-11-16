#include <stdio.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

#include "model2.h"
#include "room2.h"
#include "shader2.h"
#include "furniture2.h"

// カメラの種類
#define CAMERA_EDITOR 0
#define CAMERA_PLAYER 1

// ベクトルの正規化
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

// 内積
static float Dot(const float a[3], const float b[3])
{
    return a[0] * b[0] +
           a[1] * b[1] +
           a[2] * b[2];
}

// 外積
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

// 透視投影行列
static void CreatePerspective(
    float matrix[16],
    float fov,
    float aspect,
    float nearPlane,
    float farPlane
)
{
    const float PI = 3.14159265f;

    float f = 1.0f / tanf(fov * PI / 360.0f);

    for (int i = 0; i < 16; i++) {
        matrix[i] = 0.0f;
    }

    matrix[0] = f / aspect;
    matrix[5] = f;
    matrix[10] = (farPlane + nearPlane) /
                 (nearPlane - farPlane);
    matrix[11] = -1.0f;
    matrix[14] = (2.0f * farPlane * nearPlane) /
                 (nearPlane - farPlane);
}

// カメラ行列
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

/// カメラの視点を更新
static void UpdateCamera(
    float view[16],
    int cameraMode,
    float playerX,
    float playerY,
    float playerZ
)
{
    float eye[3];
    float center[3];
    float up[3] = {0.0f, 1.0f, 0.0f};

    if (cameraMode == CAMERA_EDITOR) {

        // マップ全体を見る俯瞰カメラ
        eye[0] = 25.0f;
        eye[1] = 32.0f;
        eye[2] = 38.0f;

        center[0] = 0.0f;
        center[1] = 0.0f;
        center[2] = 0.0f;

    } else {

        // ゴーストの後ろ斜め上から見る三人称カメラ
        eye[0] = playerX + 2.0f;
        eye[1] = playerY + 2.0f;
        eye[2] = playerZ + 0.01f;

        // ゴーストを見る
        center[0] = playerX;
        center[1] = playerY + 1.5f;
        center[2] = playerZ;
    }

    CreateLookAt(view, eye, center, up);
}
int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    // 乱数の初期化（ゲーム起動時に1回だけ）
    srand((unsigned int)time(NULL));

    // SDLの初期化
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL初期化失敗: %s\n", SDL_GetError());
        return 1;
    }

    // OpenGLのバージョン設定
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    // ウィンドウの作成
    SDL_Window *window = SDL_CreateWindow(
        "HAUNTED HEIST",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1100,
        720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    if (window == NULL) {
        printf("ウィンドウ作成失敗: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // OpenGLのコンテキスト作成
    SDL_GLContext context = SDL_GL_CreateContext(window);

    if (context == NULL) {
        printf("OpenGL初期化失敗: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // GLEWの初期化
    GLenum glewResult = glewInit();

    if (glewResult != GLEW_OK) {
        printf("GLEW初期化失敗: %s\n",
               glewGetErrorString(glewResult));

        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    printf("GLEWの初期化成功\n");

    SDL_GL_SetSwapInterval(1);

    glClearColor(0.05f, 0.10f, 0.08f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    printf("HAUNTED HEIST 起動\n");

    // シェーダーの読み込み
    unsigned int shaderProgram = LoadShaders(
        "shaders/vertex.glsl",
        "shaders/fragment.glsl"
    );

    if (shaderProgram == 0) {
        printf("シェーダー読み込み失敗\n");

        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    printf("シェーダーの読み込み成功\n");

    // 部屋の初期化
    InitRooms();

    InitFurniture();

    // マップ変更タイマー
    Uint32 lastMapChange = SDL_GetTicks();

    // 30秒ごとにマップを変更
    const Uint32 MAP_CHANGE_INTERVAL = 30000;

    // モデルの読み込み
    int largeRoomID =
        LoadModel("models/mansion/oohiroma.fbx");

    int smallRoomID =
        LoadModel("models/mansion/kobeya.fbx");

    // プレイヤーモデルの読み込み
    int playerModelID = LoadModel("models/ghost.fbx");

    if (largeRoomID == 0 || smallRoomID == 0) {
        printf("モデル読み込み失敗\n");

        FreeModel(largeRoomID);
        FreeModel(smallRoomID);
        FreeShaders(shaderProgram);

        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // プレイヤーの初期位置
    float playerX = 0.0f;
    float playerY = 0.0f;
    float playerZ = 0.0f;

    // プレイヤーモデルの大きさ
    float playerScale = 1.0f;

    // 部屋の情報を確認
    for (int i = 0; i < ROOM_COUNT; i++) {
        Room room = GetRoom(i);

        printf("部屋%d: X=%.1f Z=%.1f 幅=%.1f 奥行=%.1f\n",
               room.roomID,
               room.x,
               room.z,
               room.width,
               room.depth);
    }

    // カメラ設定
    float view[16];
    float projection[16];

    // 最初は制作・確認用カメラ
    int cameraMode = CAMERA_EDITOR;

    CreatePerspective(
        projection,
        60.0f,
        1100.0f / 720.0f,
        0.1f,
        200.0f
    );

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

    // メインループ
    int running = 1;
    SDL_Event event;

    // 前回の時間を記録
    Uint32 previousTime = SDL_GetTicks();

    while (running) {

        // イベント処理
        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_QUIT) {
                running = 0;
            }

            // Xキーでカメラ切り替え
            if (event.type == SDL_KEYDOWN &&
                event.key.repeat == 0 &&
                event.key.keysym.sym == SDLK_x) {

                if (cameraMode == CAMERA_EDITOR) {
                    cameraMode = CAMERA_PLAYER;
                    printf("ゲーム用カメラに切り替え\n");
                } else {
                    cameraMode = CAMERA_EDITOR;
                    printf("制作・確認用カメラに切り替え\n");
                }
            }
        }

        // 30秒経過したか確認
        // 経過時間を計算
        Uint32 currentTime = SDL_GetTicks();

        float deltaTime = (currentTime - previousTime) / 1000.0f;

        previousTime = currentTime;

        // キーボード入力を取得
        const Uint8 *keys = SDL_GetKeyboardState(NULL);

        // 移動方向
        float moveX = 0.0f;
        float moveZ = 0.0f;

        // 方向キー操作
        if (keys[SDL_SCANCODE_UP])    moveX -= 1.0f;
        if (keys[SDL_SCANCODE_DOWN])  moveX += 1.0f;
        if (keys[SDL_SCANCODE_LEFT])  moveZ += 1.0f;
        if (keys[SDL_SCANCODE_RIGHT]) moveZ -= 1.0f;

        // 移動速度
        float moveSpeed = 5.0f;

        // 斜め移動の速度調整
        float moveLength = sqrtf(
            moveX * moveX + moveZ * moveZ
        );

        if (moveLength > 0.0f) {
            playerX += (moveX / moveLength) * moveSpeed * deltaTime;
            playerZ += (moveZ / moveLength) * moveSpeed * deltaTime;
        }

        if (currentTime - lastMapChange >= MAP_CHANGE_INTERVAL) {

            printf("\n===== マップを変更します =====\n");

            // 小部屋の配置をランダム変更
            RandomizeRooms();

            // 新しい座標を確認
            for (int i = 0; i < ROOM_COUNT; i++) {

                Room room = GetRoom(i);

                printf(
                    "部屋%d: X=%.1f Z=%.1f\n",
                    room.roomID,
                    room.x,
                    room.z
                );
            }

            // タイマーを更新
            lastMapChange = currentTime;
        }

        // カメラの更新
        UpdateCamera(view, cameraMode, playerX, playerY, playerZ);

        glUseProgram(shaderProgram);

        glUniformMatrix4fv(
            glGetUniformLocation(shaderProgram, "view"),
            1,
            GL_FALSE,
            view
        );

        // 画面のクリア
        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT
        );

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

        // プレイヤーモデルの描画
        if (playerModelID != 0) {
            DrawModelAt(
                playerModelID,
                shaderProgram,
                playerX,
                playerY,
                playerZ,
                playerScale
            );
        }

        SDL_GL_SwapWindow(window);
    }

    // モデルの解放
    FreeModel(largeRoomID);
    FreeModel(smallRoomID);
    FreeModel(playerModelID);

    // シェーダーの解放
    FreeShaders(shaderProgram);

    // 終了処理
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
