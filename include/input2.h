#ifndef INPUT2_H
#define INPUT2_H

#include <SDL2/SDL.h>

// イベント処理
void HandleInputEvents(
    int *running,
    int *mapMode,
    int *spacePressed
);

// キーボード入力の取得
const Uint8 *GetInputKeys(void);

// カメラ操作
void UpdateCameraInput(
    const Uint8 *keys,
    int cameraMode,
    float deltaTime,
    float *cameraYaw,
    float *cameraPitch
);

// キーが押された瞬間を検知
int WasKeyPressed(SDL_Keycode key);

int WasKeyPressedInWindow(SDL_Keycode key, Uint32 windowID);
#endif
