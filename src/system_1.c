/*
 *  ファイル名 ： system1.c
 *  機能	   ： ゲームシステム処理
 */
#include "system1.h"

/* 変数 */

/* 関数 */

/* システム終了処理 */
void DestroySystem(void)
{

}

//* プレイヤー処理モジュール *//
/* 
    関数名： UpdatePlayer()
    機能  ： プレイヤーの状態を更新する
    引数  ： player   ：プレイヤー情報
             deltaTime：前フレームからの経過時間
    返り値： なし
*/
void UpdatePlayer(Player *player, float deltaTime){
    // 通報された場合
    if (!player->isActive) {
        player->stunTime -= deltaTime;
        if (player->stunTime <= 0.0f) {
            player->isActive = 1; // 行動可能に戻る
        }
        return;
    }
}

/* 
    関数名： MovePlayer()
    機能  ： プレイヤーを移動させる
    引数  ： player   ：プレイヤー情報
             inputX   ：スティックからのX軸方向の入力値
             inputY   ：スティックからのY軸方向の入力値
             deltaTime：前フレームからの経過時間
    返り値： なし
*/
void MovePlayer(Player *player, float inputX, float inputY, float deltaTime){
    if (!player->isActive) return; // 操作不能時は移動禁止 (GT04-09)

    // 斜め移動時の速度調整（ベクトルの正規化） (GT04-03)
    float length = sqrtf(inputX * inputX + inputY * inputY);
    if (length > 0.0f) {
        inputX /= length;
        inputY /= length;
    }

    float moveSpeed = 200.0f; // 移動速度設定 (GT04-01)
    player->positionX += inputX * moveSpeed * deltaTime;
    player->positionY += inputY * moveSpeed * deltaTime;

    // 壁やマップ端の衝突判定 (GT04-05)
    if (player->positionX < 0.0f) player->positionX = 0.0f;
    if (player->positionX > MAP_WIDTH) player->positionX = MAP_WIDTH;
    if (player->positionY < 0.0f) player->positionY = 0.0f;
    if (player->positionY > MAP_HEIGHT) player->positionY = MAP_HEIGHT;
}

/* 
    関数名： SetPlayerStun()
    機能  ： プレイヤーを行動不能状態にする
    引数  ： player  ：プレイヤー情報
             duration：行動不能時間
    返り値： なし
*/
void SetPlayerStun(Player *player, float duration){
    player->isActive = 0;       // 行動不可
    player->stunTime = duration; // スタン残り時間設定
    // スタート地点（屋敷の入口付近など）へ強制送還
    player->positionX = (float)(MAP_WIDTH / 2);
    player->positionY = (float)(MAP_HEIGHT / 2);
}

/* 
    関数名： UpdateCamera()
    機能  ： プレイヤーを行動不能状態にする
    引数  ： player  ：プレイヤー情報
             duration：行動不能時間
    返り値： なし
*/
void UpdateCamera(Camera *cam, Player *player) {
    // プレイヤーの中心をカメラの基準にする
    float targetX = player->positionX - (WD_WIDTH / 2.0f);
    float targetY = player->positionY - (WD_HEIGHT / 2.0f);

    // 動きを滑らかにする
    float smoothing = 0.1f;
    cam->x += (targetX - cam->x) * smoothing;
    cam->y += (targetY - cam->y) * smoothing;

    // マップ端からはみ出さないようにクランプ
    if (cam->x < 0) cam->x = 0;
    if (cam->y < 0) cam->y = 0;
    if (cam->x > MAP_WIDTH - WD_WIDTH) cam->x = MAP_WIDTH - WD_WIDTH;
    if (cam->y > MAP_HEIGHT - WD_HEIGHT) cam->y = MAP_HEIGHT - WD_HEIGHT;
}

//* 通報処理モジュール *//
/* 
    関数名： ReportTarget()
    機能  ： 指定した対象を通報する
    引数  ： 通報者番号、対象番号、対象の種類
    返り値： なし
*/
void ReportTarget(Player *reporter, Player *target){
    // 通報者が正しく対象を指している場合の処理
    ProcessReport(target, 5.0f); // 5秒間のスタン（行動不能）を与える
}

/* 
    関数名： ProcessReport()
    機能  ： 通報結果に応じて行動不能状態を設定する
    引数  ： 通報情報
    返り値： なし
*/
void ProcessReport(Player *target, float stunDuration){
    // 通報されたプレイヤーはスタート地点に戻され、一定時間行動不能になる
    SetPlayerStun(target, stunDuration);
}

/* end of system1.c */
