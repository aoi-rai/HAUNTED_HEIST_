#ifndef INPUT2_H
#define INPUT2_H

#include <SDL2/SDL.h>

// イベント処理
void HandleInputEvents(
    int *running,
    int *mapMode
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

#endif
