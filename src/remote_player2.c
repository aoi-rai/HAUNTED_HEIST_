#include <stddef.h>
#include "remote_player2.h"
#include "model2.h"

// 他プレイヤーの管理配列
static RemotePlayer remotePlayers[REMOTE_PLAYER_COUNT];

// 初期化
void InitRemotePlayers(void)
{
    for (int i = 0; i < REMOTE_PLAYER_COUNT; i++) {
        remotePlayers[i].playerID = -1;
        remotePlayers[i].x = 0.0f;
        remotePlayers[i].y = 0.0f;
        remotePlayers[i].z = 0.0f;
        remotePlayers[i].direction = 0.0f;
        remotePlayers[i].connected = 0;
    }
}

// 他プレイヤーの情報を登録・更新
int SetRemotePlayerState(
    int playerID,
    float x,
    float y,
    float z,
    float direction
)
{
    // 無効なプレイヤーIDを拒否
    if (playerID < 0) {
        return 0;
    }

    int slot = -1;

    // 登録済みプレイヤーを探す
    for (int i = 0; i < REMOTE_PLAYER_COUNT; i++) {
        if (remotePlayers[i].connected &&
            remotePlayers[i].playerID == playerID) {
            slot = i;
            break;
        }
    }

    // 未登録なら空き領域を探す
    if (slot == -1) {
        for (int i = 0; i < REMOTE_PLAYER_COUNT; i++) {
            if (!remotePlayers[i].connected) {
                slot = i;
                break;
            }
        }
    }

    // 空きがない
    if (slot == -1) {
        return 0;
    }

    remotePlayers[slot].playerID = playerID;
    remotePlayers[slot].x = x;
    remotePlayers[slot].y = y;
    remotePlayers[slot].z = z;
    remotePlayers[slot].direction = direction;
    remotePlayers[slot].connected = 1;

    return 1;
}

// プレイヤーIDを指定して切断
int RemoveRemotePlayer(int playerID)
{
    for (int i = 0; i < REMOTE_PLAYER_COUNT; i++) {
        if (remotePlayers[i].connected &&
            remotePlayers[i].playerID == playerID) {

            remotePlayers[i].playerID = -1;
            remotePlayers[i].x = 0.0f;
            remotePlayers[i].y = 0.0f;
            remotePlayers[i].z = 0.0f;
            remotePlayers[i].direction = 0.0f;
            remotePlayers[i].connected = 0;

            return 1;
        }
    }

    return 0;
}

// 配列の番号で取得
const RemotePlayer *GetRemotePlayer(int index)
{
    if (index < 0 || index >= REMOTE_PLAYER_COUNT) {
        return 0;
    }

    if (!remotePlayers[index].connected) {
        return 0;
    }

    return &remotePlayers[index];
}

// プレイヤーIDで取得
const RemotePlayer *FindRemotePlayer(int playerID)
{
    for (int i = 0; i < REMOTE_PLAYER_COUNT; i++) {
        if (remotePlayers[i].connected &&
            remotePlayers[i].playerID == playerID) {
            return &remotePlayers[i];
        }
    }

    return 0;
}

// 他プレイヤーの描画
void DrawRemotePlayers(
    int playerModelID,
    unsigned int shaderProgram,
    float playerScale
)
{
    if (playerModelID <= 0) {
        return;
    }

    for (int i = 0; i < REMOTE_PLAYER_COUNT; i++) {
        const RemotePlayer *player = GetRemotePlayer(i);

        if (player == NULL) {
            continue;
        }

        DrawModelAtRotated(
            playerModelID,
            shaderProgram,
            player->x,
            player->y,
            player->z,
            playerScale,
            player->direction
        );
    }
}