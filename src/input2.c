#include <stdio.h>
#include <SDL2/SDL.h>
#include "input2.h"
#include "camera2.h"

// イベント処理
void HandleInputEvents(
    int *running,
    int *mapMode
)
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT) {
            *running = 0;
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
