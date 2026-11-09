
#include <stdio.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>

#include "model2.h"
#include "room2.h"

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

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

    // メインループ
    int running = 1;
    SDL_Event event;

    while (running) {

        // イベント処理
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

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

