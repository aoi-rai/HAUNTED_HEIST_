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

// ウィンドウ・OpenGLの初期化
int InitWindow(WindowContext *app);

// ウィンドウ・SDLの終了処理
void CloseWindow(WindowContext *app);

#endif
