#ifndef REMOTE_PLAYER2_H
#define REMOTE_PLAYER2_H

#define REMOTE_PLAYER_COUNT 3

typedef struct {
    int playerID;
    float x;
    float y;
    float z;
    float direction;

    /*受信した目標位置*/
    float targetX;
    float targetY;
    float targetZ;
    float targetDirection;

    int connected;
} RemotePlayer;

void InitRemotePlayers(void);
void UpdateRemotePlayers(float deltaTime);

int SetRemotePlayerState(
    int playerID,
    float x,
    float y,
    float z,
    float direction
);

int RemoveRemotePlayer(int playerID);

const RemotePlayer *GetRemotePlayer(int index);
const RemotePlayer *FindRemotePlayer(int playerID);

void DrawRemotePlayers(
    int playerModelID,
    unsigned int shaderProgram,
    float playerScale
);

#endif
