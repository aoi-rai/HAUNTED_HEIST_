#include <math.h>
#include <SDL2/SDL.h>

#include "player2.h"

#define CAMERA_PLAYER 1
#define PI 3.14159265f

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
) {
    // カメラの向きにプレイヤーの向きを合わせる
    *playerTargetDirection = atan2f(
        -cosf(cameraYaw),
        -sinf(cameraYaw)
    );

    // 方向キーによる移動
    float moveX = 0.0f;
    float moveZ = 0.0f;

    // カメラの向いている方向
    float forwardX = -sinf(cameraYaw);
    float forwardZ = -cosf(cameraYaw);

    // カメラから見た右方向
    float rightX = cosf(cameraYaw);
    float rightZ = -sinf(cameraYaw);

    if (cameraMode == CAMERA_PLAYER) {
        if (keys[SDL_SCANCODE_UP]) {
            moveX += forwardX;
            moveZ += forwardZ;
        }

        if (keys[SDL_SCANCODE_DOWN]) {
            moveX -= forwardX;
            moveZ -= forwardZ;
        }

        if (keys[SDL_SCANCODE_LEFT]) {
            moveX -= rightX;
            moveZ -= rightZ;
        }

        if (keys[SDL_SCANCODE_RIGHT]) {
            moveX += rightX;
            moveZ += rightZ;
        }
    }

    // 移動速度
    float moveSpeed = 5.0f;

    // 斜め移動の速度調整
    float moveLength = sqrtf(
        moveX * moveX +
        moveZ * moveZ
    );

    if (moveLength > 0.0f) {
        *playerX +=
            (moveX / moveLength) *
            moveSpeed *
            deltaTime;

        *playerZ +=
            (moveZ / moveLength) *
            moveSpeed *
            deltaTime;

        // 移動方向を目標角度にする
        *playerTargetDirection = atan2f(
            -moveZ,
            moveX
        );
    }

    // プレイヤーの滑らかな旋回
    {
        const float TWO_PI = 6.28318530f;

        float angleDifference =
            *playerTargetDirection - *playerDirection;

        while (angleDifference > PI) {
            angleDifference -= TWO_PI;
        }

        while (angleDifference < -PI) {
            angleDifference += TWO_PI;
        }

        float maxTurn = playerRotationSpeed * deltaTime;

        if (fabsf(angleDifference) <= maxTurn) {
            *playerDirection = *playerTargetDirection;
        } else if (angleDifference > 0.0f) {
            *playerDirection += maxTurn;
        } else {
            *playerDirection -= maxTurn;
        }

        // 角度を -PI ～ PI に収める
        while (*playerDirection > PI) {
            *playerDirection -= TWO_PI;
        }

        while (*playerDirection < -PI) {
            *playerDirection += TWO_PI;
        }
    }
}
