#include <stdio.h>

#include "model2.h"
#include "model_init2.h"

int InitGameModels(
    int *largeRoomID,
    int *smallRoomID,
    int *playerModelID
) {
    // 大広間モデル
    *largeRoomID =
        LoadModel("models/mansion/oohiroma.fbx");

    // 小部屋モデル
    *smallRoomID =
        LoadModel("models/mansion/kobeya.fbx");

    // プレイヤーモデル
    *playerModelID =
        LoadModel("models/ghost.fbx");

    // 読み込み結果を確認
    if (*largeRoomID == 0 ||
        *smallRoomID == 0 ||
        *playerModelID == 0) {

        printf("モデル読み込み失敗\n");
        return 0;
    }

    printf("ゲームモデルの読み込み成功\n");
    return 1;
}
