#ifndef GAME_INIT2_H
#define GAME_INIT2_H

typedef struct {

    // プレイヤー情報
    float playerX;
    float playerY;
    float playerZ;
    float playerScale;
    float playerDirection;
    float playerTargetDirection;
    float playerRotationSpeed;

    // カメラ情報
    float view[16];
    float projection[16];
    int cameraMode;
    float cameraYaw;
    float cameraPitch;

    // マップ情報
    int mapMode;

    // NPC選択情報
    int selectedNPCID;
    int previousSelectedNPCID;

} GameInitData;

int InitGameWorld(void);

void InitGameState(
    GameInitData *game,
    unsigned int shaderProgram
);

#endif
