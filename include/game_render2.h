#ifndef GAME_RENDER2_H
#define GAME_RENDER2_H

void DrawGameScene(
    int largeRoomID,
    int smallRoomID,
    int playerModelID,
    unsigned int shaderProgram,
    int cameraMode,
    int selectedNPCID,
    float playerX,
    float playerY,
    float playerZ,
    float playerScale,
    float playerDirection
);

void DrawWaitingBackground(unsigned int shaderProgram);

void DrawBlindnessOverlay(float blindnessTimer);

#endif
