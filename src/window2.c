#include <stdio.h>
#include <GL/glew.h>
#include <SDL2/SDL_image.h>

#include "window2.h"

void CloseWindow(WindowContext *app)
{
    if (app == NULL) {
        return;
    }

    if (app->context != NULL) {
        SDL_GL_DeleteContext(app->context);
        app->context = NULL;
    }

    if (app->window != NULL) {
        SDL_DestroyWindow(app->window);
        app->window = NULL;
    }

    if (app->imageInitialized) {
        IMG_Quit();
        app->imageInitialized = 0;
    }

    if (app->sdlInitialized) {
        SDL_Quit();
        app->sdlInitialized = 0;
    }
}

int InitWindow(WindowContext *app)
{
    if (app == NULL) {
        return 0;
    }

    app->window = NULL;
    app->context = NULL;
    app->sdlInitialized = 0;
    app->imageInitialized = 0;

    // SDLの初期化
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL初期化失敗: %s\n", SDL_GetError());
        return 0;
    }

    app->sdlInitialized = 1;

    // PNG画像読み込みの初期化
    int imageResult = IMG_Init(IMG_INIT_PNG);
    app->imageInitialized = 1;

    if ((imageResult & IMG_INIT_PNG) == 0) {
        printf("SDL_image初期化失敗: %s\n", IMG_GetError());
        CloseWindow(app);
        return 0;
    }

    printf("PNG画像読み込みの初期化成功\n");

    // OpenGLの設定
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    // ウィンドウの作成
    app->window = SDL_CreateWindow(
        "HAUNTED HEIST",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1000,
        720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    if (app->window == NULL) {
        printf("ウィンドウ作成失敗: %s\n", SDL_GetError());
        CloseWindow(app);
        return 0;
    }

    // OpenGLコンテキスト作成
    app->context = SDL_GL_CreateContext(app->window);

    if (app->context == NULL) {
        printf("OpenGL初期化失敗: %s\n", SDL_GetError());
        CloseWindow(app);
        return 0;
    }

    // GLEWの初期化
    GLenum glewResult = glewInit();

    if (glewResult != GLEW_OK) {
        printf(
            "GLEW初期化失敗: %s\n",
            glewGetErrorString(glewResult)
        );

        CloseWindow(app);
        return 0;
    }

    printf("GLEWの初期化成功\n");

    // OpenGLの基本設定
    SDL_GL_SetSwapInterval(1);

    glViewport(0, 0, 1000, 720);
    glClearColor(0.05f, 0.10f, 0.08f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    printf("HAUNTED HEIST 起動\n");

    return 1;
}

// ========================================
// 共有OpenGLコンテキストを使った待機用ウィンドウ
// ========================================
int InitSharedWindow(
    WindowContext *primary,
    WindowContext *secondary
)
{
    if (primary == NULL || secondary == NULL ||
        primary->window == NULL || primary->context == NULL) {
        return 0;
    }

    secondary->window = NULL;
    secondary->context = NULL;
    secondary->sdlInitialized = 0;
    secondary->imageInitialized = 0;

    // メインのOpenGLコンテキストを有効にする
    if (SDL_GL_MakeCurrent(primary->window, primary->context) < 0) {
        printf("メインコンテキスト切り替え失敗: %s\n",
               SDL_GetError());
        return 0;
    }

    // OpenGLリソースを共有する設定
    SDL_GL_SetAttribute(
        SDL_GL_SHARE_WITH_CURRENT_CONTEXT,
        1
    );

    secondary->window = SDL_CreateWindow(
        "HAUNTED HEIST - WAITING",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1000,
        720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    if (secondary->window == NULL) {
        printf("待機ウィンドウ作成失敗: %s\n",
               SDL_GetError());

        SDL_GL_SetAttribute(
            SDL_GL_SHARE_WITH_CURRENT_CONTEXT,
            0
        );
        return 0;
    }

    secondary->context =
        SDL_GL_CreateContext(secondary->window);

    // 次回以降のコンテキスト作成に影響しないよう戻す
    SDL_GL_SetAttribute(
        SDL_GL_SHARE_WITH_CURRENT_CONTEXT,
        0
    );

    if (secondary->context == NULL) {
        printf("待機用OpenGL初期化失敗: %s\n",
               SDL_GetError());

        SDL_DestroyWindow(secondary->window);
        secondary->window = NULL;

        return 0;
    }

    // 待機用コンテキストに切り替える
    if (SDL_GL_MakeCurrent(
        secondary->window,
        secondary->context
    ) < 0) {
        printf("待機用コンテキスト切り替え失敗: %s\n",
               SDL_GetError());

        SDL_GL_DeleteContext(secondary->context);
        SDL_DestroyWindow(secondary->window);

        secondary->context = NULL;
        secondary->window = NULL;

        SDL_GL_MakeCurrent(primary->window, primary->context);
        return 0;
    }

    // 待機用ウィンドウのOpenGL状態
    SDL_GL_SetSwapInterval(1);

    glViewport(0, 0, 1000, 720);
    glClearColor(0.05f, 0.10f, 0.08f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    // メインコンテキストに戻す
    SDL_GL_MakeCurrent(primary->window, primary->context);

    printf("待機用ウィンドウの作成成功\n");

    return 1;
}


// ========================================
// 待機用ウィンドウだけを終了
// ========================================
void CloseSharedWindow(
    WindowContext *primary,
    WindowContext *secondary
)
{
    if (secondary == NULL) {
        return;
    }

    // メインコンテキストに戻してから待機用を破棄
    if (primary != NULL &&
        primary->window != NULL &&
        primary->context != NULL) {

        SDL_GL_MakeCurrent(
            primary->window,
            primary->context
        );
    }

    if (secondary->context != NULL) {
        SDL_GL_DeleteContext(secondary->context);
        secondary->context = NULL;
    }

    if (secondary->window != NULL) {
        SDL_DestroyWindow(secondary->window);
        secondary->window = NULL;
    }

    printf("待機用ウィンドウを終了しました\n");
}