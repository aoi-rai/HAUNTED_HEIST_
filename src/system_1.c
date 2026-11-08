/*
 *  ファイル名 ： system1.c
 *  機能	   ： ゲームシステム処理
 */
#include "system1.h"

/* 変数 */

/* 関数 */

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
    機能  ： プレイヤーを移動させ、移動方向（向き）を更新する
    引数  ： player   ：プレイヤー情報
             inputX   ：スティックからのX軸方向の入力値
             inputY   ：スティックからのY軸方向の入力値
             deltaTime：前フレームからの経過時間
    返り値： なし
*/
void MovePlayer(Player *player, float inputX, float inputY, float deltaTime){
    if (!player->isActive) return; // 操作不能時は移動禁止

    // 斜め移動時の速度調整（ベクトルの正規化）
    float length = sqrtf(inputX * inputX + inputY * inputY);
    if (length > 0.0f) {
        inputX /= length;
        inputY /= length;

        // 入力がある場合、プレイヤーの向きを更新する
        player->directionX = inputX;
        player->directionY = inputY;
    }

    float moveSpeed = 200.0f; // 移動速度設定
    player->positionX += inputX * moveSpeed * deltaTime;
    player->positionY += inputY * moveSpeed * deltaTime;

    // 壁やマップ端の衝突判定
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

//* カメラ処理モジュール *//
/* 
    関数名： UpdateCamera()
    機能  ： カメラの初期化と追従・滑らかな調整
    引数  ： player     ：プレイヤー情報
             cam        ：カメラ情報
             rightStickX：右スティックのX軸入力値 (-32768 ～ 32767)
             deltaTime  ：前フレームからの経過時間
    返り値： なし
*/
void UpdateCamera(Player *player, Camera *cam, float rightStickX, float deltaTime) {
    // 回転操作の更新
    float rotationSpeed = 2.0f;
    float normalizedStick = 0.0f;
    if (rightStickX > 8000 || rightStickX < -8000) {
        normalizedStick = rightStickX / 32767.0f;
    }
    cam->angle += normalizedStick * rotationSpeed * deltaTime;

    // プレイヤーの位置を基準にしつつ、カメラの距離（distance）と角度（angle）を反映する
    float targetX = player->positionX - (WD_WIDTH / 2.0f) - cosf(cam->angle) * cam->distance;
    float targetY = player->positionY - (WD_HEIGHT / 2.0f) - sinf(cam->angle) * cam->distance;

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

/* 
    関数名： UpdateCameraRotation()
    機能  ： 右スティック入力等でカメラを回転
    引数  ： cam        ：カメラ情報
             rightStickX：右スティックのX軸入力値 (-32768 ～ 32767)
             deltaTime  ：前フレームからの経過時間
    返り値： なし
*/
void UpdateCameraRotation(Camera *cam, float rightStickX, float deltaTime) {
    // 回転速度の調整
    float rotationSpeed = 2.0f; // 必要に応じて感度を変更

    // デッドゾーン（スティックの遊び）の処理
    if (rightStickX < 8000 && rightStickX > -8000) {
        rightStickX = 0.0f;
    } else {
        // -1.0 ～ 1.0 の範囲に正規化
        rightStickX = rightStickX / 32767.0f;
    }

    // カメラの角度を更新
    cam->angle += rightStickX * rotationSpeed * deltaTime;

    // 0度～360度（2πラジアン）の範囲に正規化する場合
    if (cam->angle > 6.283185f) cam->angle -= 6.283185f;
    if (cam->angle < 0.0f) cam->angle += 6.283185f;
}

//* 通報処理モジュール *//
/* 
    関数名： ReportTarget()
    機能  ： 指定した対象を通報する
    引数  ： target：通報対象番号
    返り値： なし
*/
void ReportTarget(Player *target){
    // 通報者が正しく対象を指している場合の処理
    ProcessReport(target, 5.0f); // 5秒間のスタン（行動不能）を与える
}

/* 
    関数名： ProcessReport()
    機能  ： 通報結果に応じて行動不能状態を設定する
    引数  ： target      ：通報対象番号
             stunDuration：行動不能時間
    返り値： なし
*/
void ProcessReport(Player *target, float stunDuration){
    // 通報されたプレイヤーはスタート地点に戻され、一定時間行動不能になる
    SetPlayerStun(target, stunDuration);
}

/* end of system1.c */
