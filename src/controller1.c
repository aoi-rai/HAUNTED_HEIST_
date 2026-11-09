#include "controller1.h"
#include <stdio.h>

static SDL_GameController *gGameController = NULL;

/* コントローラーの初期化と接続確認 */
int InitController(void) {
    // イベントサブシステムに加えてゲームコントローラーサブシステムを初期化
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0) {
        printf("ゲームコントローラーの初期化に失敗しました: %s\n", SDL_GetError());
        return 0;
    }

    // 接続されているコントローラーを探して開く
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            gGameController = SDL_GameControllerOpen(i);
            if (gGameController != NULL) {
                printf("コントローラー接続成功 [%d]: %s\n", i, SDL_GameControllerName(gGameController));
                return 1;
            }
        }
    }

    printf("利用可能なゲームコントローラーが見つかりませんでした。\n");
    return 0;
}

/* コントローラーの切断・終了処理 */
void CloseController(void) {
    if (gGameController != NULL) {
        SDL_GameControllerClose(gGameController);
        gGameController = NULL;
    }
}

/* 入力値の取得（左スティック、右スティックX、ボタン） */
void PollControllerInput(float *inputX, float *inputY, float *rightStickX, int *dashPressed, int *jumpPressed) {
    if (gGameController == NULL) {
        *inputX = 0.0f;
        *inputY = 0.0f;
        *rightStickX = 0.0f;
        *dashPressed = 0;
        *jumpPressed = 0;
        return;
    }

    // 軸の値を取得 (-32768 ～ 32767)
    Sint16 leftX  = SDL_GameControllerGetAxis(gGameController, SDL_CONTROLLER_AXIS_LEFTX);
    Sint16 leftY  = SDL_GameControllerGetAxis(gGameController, SDL_CONTROLLER_AXIS_LEFTY);
    Sint16 rightX = SDL_GameControllerGetAxis(gGameController, SDL_CONTROLLER_AXIS_RIGHTX);

    // デッドゾーン（スティックの遊び）の処理
    float deadzone = 8000.0f;

    *inputX = (leftX > deadzone || leftX < -deadzone) ? (float)leftX / 32767.0f : 0.0f;
    *inputY = (leftY > deadzone || leftY < -deadzone) ? (float)leftY / 32767.0f : 0.0f;
    
    // カメラ回転用の右スティックX軸 (system1.cの UpdateCameraRotation にそのまま渡せる値)
    *rightStickX = (rightX > deadzone || rightX < -deadzone) ? (float)rightX : 0.0f;

    // ボタン状態の取得 (例: Aボタンでジャンプ、Xボタンでダッシュ)
    *jumpPressed = SDL_GameControllerGetButton(gGameController, SDL_CONTROLLER_BUTTON_A);
    *dashPressed = SDL_GameControllerGetButton(gGameController, SDL_CONTROLLER_BUTTON_X);
}