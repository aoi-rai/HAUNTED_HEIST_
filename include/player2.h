#ifndef PLAYER2_H
#define PLAYER2_H

#include <SDL2/SDL.h>

// 3Dプレイヤーの移動・旋回
void UpdatePlayerMovement(
    const Uint8 *keys,
    int cameraMode,
    float cameraYaw,
    float deltaTime,
    float playerRotationSpeed,
    float *playerX,
    float *playerZ,
    float *playerDirection,
    float *playerTargetDirection
);

#endif
