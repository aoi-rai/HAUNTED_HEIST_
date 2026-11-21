#include <stdio.h>
#include <SDL2/SDL.h>
#include <unistd.h>
#include "input2.h"
#include "camera2.h"
#define MAX_PRESSED_KEYS 16

static SDL_Keycode pressedKeys[MAX_PRESSED_KEYS];
static Uint32 pressedWindowIDs[16];
static int pressedKeyCount = 0;

// イベント処理
void HandleInputEvents(
    int *running,
    int *mapMode,
    int *spacePressed
)
{
    SDL_Event event;

    *spacePressed = 0;
    pressedKeyCount = 0;

    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_WINDOWEVENT) {

    if (
        event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED ||
        event.window.event == SDL_WINDOWEVENT_FOCUS_LOST
    ) {
        printf(
            "[FOCUS DEBUG] PID=%ld windowID=%u event=%s\n",
            (long)getpid(),
            event.window.windowID,
            event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED
                ? "FOCUS_GAINED"
                : "FOCUS_LOST"
        );
    }
}

        if (event.type == SDL_QUIT) {
            printf("[INPUT DEBUG] SDL_QUIT を受信しました。\\n");
            *running = 0;
        }

        if (event.type == SDL_KEYDOWN) {
    printf(
        "[KEY DEBUG] PID=%ld KEY=%s windowID=%u repeat=%d timestamp=%u\n",
        (long)getpid(),
        SDL_GetKeyName(event.key.keysym.sym),
        event.key.windowID,
        event.key.repeat,
        event.key.timestamp
    );
}

        // 押されたキーを記録
if (
    event.type == SDL_KEYDOWN &&
    event.key.repeat == 0 &&
    pressedKeyCount < MAX_PRESSED_KEYS
) {
    pressedKeys[pressedKeyCount] = event.key.keysym.sym;
pressedWindowIDs[pressedKeyCount] = event.key.windowID;
pressedKeyCount++;
}

        // SpaceキーでNPCにアクション
        if (
            event.type == SDL_KEYDOWN &&
            event.key.repeat == 0 &&
            event.key.keysym.sym == SDLK_SPACE
        ) {
            *spacePressed = 1;
        }

        // Xキーでマップ切り替え
        if (
            event.type == SDL_KEYDOWN &&
            event.key.repeat == 0 &&
            event.key.keysym.sym == SDLK_x
        ) {
            *mapMode = 1 - *mapMode;

            if (*mapMode == 0) {
                printf("周辺マップに切り替え\n");
            } else {
                printf("全体マップに切り替え\n");
            }
        }
    }
}

// 指定したキーが押された瞬間か確認
int WasKeyPressed(SDL_Keycode key)
{
    for (int i = 0; i < pressedKeyCount; i++) {
        if (pressedKeys[i] == key) {
            return 1;
        }
    }

    return 0;
}

int WasKeyPressedInWindow(SDL_Keycode key, Uint32 windowID)
{
    for (int i = 0; i < pressedKeyCount; i++)
    {
        if (pressedKeys[i] == key &&
            pressedWindowIDs[i] == windowID)
        {
            return 1;
        }
    }

    return 0;
}

// キーボード入力の取得
const Uint8 *GetInputKeys(void)
{
    return SDL_GetKeyboardState(NULL);
}

// カメラ操作
void UpdateCameraInput(
    const Uint8 *keys,
    int cameraMode,
    float deltaTime,
    float *cameraYaw,
    float *cameraPitch
)
{
    if (cameraMode == CAMERA_PLAYER) {

        float cameraRotateSpeed = 2.0f * deltaTime;

        if (keys[SDL_SCANCODE_D]) {
            *cameraYaw -= cameraRotateSpeed;
        }

        if (keys[SDL_SCANCODE_A]) {
            *cameraYaw += cameraRotateSpeed;
        }

        if (keys[SDL_SCANCODE_W]) {
            *cameraPitch += cameraRotateSpeed;
        }

        if (keys[SDL_SCANCODE_S]) {
            *cameraPitch -= cameraRotateSpeed;
        }

        if (*cameraPitch > 1.2f) {
            *cameraPitch = 1.2f;
        }

        if (*cameraPitch < -1.2f) {
            *cameraPitch = -1.2f;
        }
    }
}
