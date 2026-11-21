
#ifndef NPC2_H
#define NPC2_H

#define NPC_COUNT 50

#define NORMAL_GHOST_COUNT 42
#define REAL_TREASURE_GHOST_COUNT 2
#define FAKE_TREASURE_GHOST_COUNT 6

#define GHOST_NORMAL 0
#define GHOST_REAL_TREASURE 1
#define GHOST_FAKE_TREASURE 2

#define NPC_BEHAVIOR_FLOAT       0
#define NPC_BEHAVIOR_WALK        1
#define NPC_BEHAVIOR_WALK_STOP   2
#define NPC_BEHAVIOR_WALK_FLOAT  3
#define NPC_BEHAVIOR_ALL         4

typedef struct {
    float x;
    float y;
    float z;

    float targetX;
    float targetZ;

float speed;
float moveTimer;

// 現在向いている方向
float direction;

// 次に向きたい方向
float targetDirection;

    int type;
    int roomID;

    // NPCの行動パターン
    int behaviorType;
// 歩行・静止の管理
int isStopped;
float actionTimer;

int isFloating;
float floatTimer;

} NPC;

// NPCの初期化
void InitNPCs(void);

// NPCの移動更新
void UpdateNPCs(float deltaTime);

// NPCの情報を取得
NPC GetNPC(int npcID);

// NPCのお宝を取得する（成功時1、失敗時0）
int TryTakeNPC(int npcID);

// プレイヤーの初期位置を取得
void GetPlayerStartPosition(
    float *x,
    float *y,
    float *z
);

// 部屋の移動に合わせてNPCを移動
void MoveNPCsWithRoom(
    int roomID,
    float dx,
    float dz
);

int FindTargetNPC(
    int cameraMode,
    float playerX,
    float playerY,
    float playerZ,
    float cameraYaw,
    float cameraPitch
);

// NPCの描画
void DrawNPCs(
    int modelID,
    unsigned int shaderProgram,
    int selectedNPCID,
    float scale
);

#endif
