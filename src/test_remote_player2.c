#include <stdio.h>
#include "remote_player2.h"

int main(void)
{
    InitRemotePlayers();

    printf("===== 他プレイヤー管理テスト =====\n");

    // プレイヤー登録
    printf("登録テスト: ");
    if (SetRemotePlayerState(1, 10.0f, 0.0f, 20.0f, 1.5f)) {
        printf("成功\n");
    } else {
        printf("失敗\n");
        return 1;
    }

    // 位置情報取得
    printf("取得テスト: ");
    const RemotePlayer *player = FindRemotePlayer(1);

    if (player != NULL) {
        printf(
            "ID=%d X=%.1f Y=%.1f Z=%.1f\n",
            player->playerID,
            player->x,
            player->y,
            player->z
        );
    } else {
        printf("失敗\n");
        return 1;
    }

    // 位置更新
    printf("更新テスト: ");
    SetRemotePlayerState(1, 30.0f, 0.0f, 40.0f, 2.0f);

    player = FindRemotePlayer(1);

    if (player != NULL && player->x == 30.0f && player->z == 40.0f) {
        printf("成功\n");
    } else {
        printf("失敗\n");
        return 1;
    }

    // 切断
    printf("切断テスト: ");
    RemoveRemotePlayer(1);

    if (FindRemotePlayer(1) == NULL) {
        printf("成功\n");
    } else {
        printf("失敗\n");
        return 1;
    }

    printf("===== 全テスト完了 =====\n");

    return 0;
}
