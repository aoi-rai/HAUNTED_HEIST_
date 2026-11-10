
#include <stdio.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>

#include "model2.h"
#include "room2.h"
#include "controller1.h"
#include "system1.h"

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    // SDLの初期化
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL初期化失敗: %s\n", SDL_GetError());
        return 1;
    }

    // コントローラーの初期化
    InitController();

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
        CloseController();
        SDL_Quit();
        return 1;
    }

    // OpenGLのコンテキスト作成
    SDL_GLContext context = SDL_GL_CreateContext(window);

    if (context == NULL) {
        printf("OpenGL初期化失敗: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        CloseController();
        SDL_Quit();
        return 1;
    }

    // 垂直同期
    SDL_GL_SetSwapInterval(1);

    // 背景色の設定
    glClearColor(0.05f, 0.10f, 0.08f, 1.0f);

    printf("HAUNTED HEIST 起動\n");

    // 大広間モデルの読み込み確認
    LoadModel("models/mansion/oohiroma.fbx");

    // 小部屋モデルの読み込み確認
    LoadModel("models/mansion/kobeya.fbx");

    // 部屋の初期化
    InitRooms();

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

    // プレイヤーとカメラの初期化
    Player player = {
        .playerdId = 1,
        .positionX = (float)(MAP_WIDTH / 2),
        .positionY = (float)(MAP_HEIGHT / 2),
        .positionZ = 0.0f,
        .isActive = 1,
        .dashCooldown = 0.0f
    };

    // メインループ用の時間計測変数
    Uint32 lastTime = SDL_GetTicks();

    // メインループ
    int running = 1;
    SDL_Event event;

    while (running) {

        // デルタタイム（経過時間）の計算
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        if (deltaTime > 0.1f) deltaTime = 0.1f; // 極端なフレーム落ち防止

        // イベント処理
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        // コントローラー入力の取得
        float inputX = 0.0f, inputY = 0.0f, rightStickX = 0.0f;
        int dashPressed = 0, jumpPressed = 0;
        PollControllerInput(&inputX, &inputY, &rightStickX, &dashPressed, &jumpPressed);

        // プレイヤー・アクションの更新
        UpdatePlayer(&player, deltaTime);

        // ダッシュ・ジャンプのトリガー入力処理
        if (dashPressed) {
            StartDash(&player);
        }
        if (jumpPressed) {
            TriggerJump(&player);
        }

        // プレイヤーの移動処理
        MovePlayer(&player, inputX, inputY, deltaTime);

        // カメラの更新処理
        Camera *cam;
        UpdateCamera(&player, &cam, rightStickX, deltaTime);

        // 画面の描画

        // 画面の描画
        glClear(GL_COLOR_BUFFER_BIT);

        // 画面の更新
        SDL_GL_SwapWindow(window);
    }

    // 終了処理
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

