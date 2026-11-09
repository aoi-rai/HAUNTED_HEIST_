/*
 *  ファイル名 ： treasure1.c
 *  機能	   ： 宝箱処理
 */
#include "system1.h"

/* 変数 */

/* 
    関数名： InitializeChests()
    機能  ： 宝箱を初期配置する
    引数  ： なし
    返り値： なし
*/
void InitializeChests(TreasureChest *chests, int maxChests){
    for (int i = 0; i < maxChests; i++) {
        chests[i].chestId = i;
        // マップ内のランダムな位置に配置
        chests[i].positionX = (float)(rand() % (MAP_WIDTH - 100) + 50);
        chests[i].positionY = (float)(rand() % (MAP_HEIGHT - 100) + 50);
        chests[i].chestType = (i == 0) ? 1 : 0; // 例: 0番を本物のお宝とする
        chests[i].isOpened = 0;
        chests[i].ownerId = -1;
    }
}

/* 
    関数名： TakeChest()
    機能  ： 宝箱を取得する
    引数  ： プレイヤー番号、宝箱番号
    返り値： 成功・失敗
*/
int TakeChest(Player *player, TreasureChest *chest){
    // 開封済みであり、まだ誰も所持していない場合
    if (chest->isOpened && chest->ownerId == -1) {
        chest->ownerId = player->playerdId;
        if (chest->chestType == 1) {
            player->hasTreasure = 1; // 本物のお宝を入手
        } else {
            player->haskey = 1;      // 鍵を入手
        }
        return 1; // 成功
    }
    return 0; // 失敗
}

/* 
    関数名： OpenChest()
    機能  ： 宝箱を開封する
    引数  ： プレイヤー番号、宝箱番号
    返り値： 宝箱の中身
*/
int OpenChest(Player *player, TreasureChest *chest){
    // プレイヤーが宝箱の近くにいるかどうかの判定をここに含めるとより安全です
    if (!chest->isOpened) {
        chest->isOpened = 1;
        return chest->chestType; // 宝箱の種類（中身）を返す
    }
    return -1; // すでに開封済み
}

/* end of treasure1.c */
