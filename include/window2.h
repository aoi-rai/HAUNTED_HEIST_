#ifndef WINDOW2_H
#define WINDOW2_H

#include <SDL2/SDL.h>

// ウィンドウとOpenGLの管理情報
typedef struct {
    SDL_Window *window;
    SDL_GLContext context;

    int sdlInitialized;
    int imageInitialized;
} WindowContext;

// メインウィンドウの初期化
int InitWindow(WindowContext *app);

// メインウィンドウを終了
void CloseWindow(WindowContext *app);

// 共有OpenGLコンテキストを使った待機用ウィンドウの作成
int InitSharedWindow(
    WindowContext *primary,
    WindowContext *secondary
);

// 待機用ウィンドウだけを終了
void CloseSharedWindow(
    WindowContext *primary,
    WindowContext *secondary
);

#endif
