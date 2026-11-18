#include <stdio.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

#include "model2.h"
#include "room2.h"
#include "shader2.h"
#include "furniture2.h"
#include "npc2.h"
#include "map_random2.h"
#include "camera2.h"
#include "map_display2.h"
#include "environment2.h"
#include "player2.h"
#include "map_update2.h"
#include "game_render2.h"
#include "game_init2.h"
#include "model_init2.h"
#include "window2.h"
#include "input2.h"
#include "target2.h"
#include "game_update2.h"
#include "ui2.h"
#include "game_cleanup2.h"
#include "remote_player2.h"
#include "network.h"

// カメラの種類
#define CAMERA_EDITOR 0
#define CAMERA_PLAYER 1

#define PI 3.14159265f

// ----------------------------------------
// ベクトルの正規化
// ----------------------------------------
int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    // 乱数の初期化
    srand((unsigned int)time(NULL));


    // ------------------------------------
    // ウィンドウ・OpenGLの初期化
    WindowContext appWindow;

    if (!InitWindow(&appWindow)) {
        return 1;
    }

    // 既存処理との互換性のため変数名を維持
    SDL_Window *window = appWindow.window;

    // ------------------------------------
    // シェーダーの読み込み
    // ------------------------------------
    unsigned int shaderProgram = LoadShaders(
        "shaders/vertex.glsl",
        "shaders/fragment.glsl"
    );

    if (shaderProgram == 0) {
        printf("シェーダー読み込み失敗\n");

        CloseWindow(&appWindow);
        return 1;
    }

    printf("シェーダーの読み込み成功\n");

    // ------------------------------------
    // マップ変更タイマー
    // ------------------------------------
    Uint32 lastMapChange = SDL_GetTicks();

    const Uint32 MAP_CHANGE_INTERVAL = 30000;

    // ------------------------------------
    // モデルの読み込み
    int largeRoomID = 0;
    int smallRoomID = 0;
    int playerModelID = 0;

    if (!InitGameModels(
        &largeRoomID,
        &smallRoomID,
        &playerModelID
    )) {
        FreeModel(largeRoomID);
        FreeModel(smallRoomID);
        FreeModel(playerModelID);
        FreeShaders(shaderProgram);

        CloseWindow(&appWindow);
        return 1;
    }

    // ------------------------------------
    // ゲームワールド全体の初期化
    // ------------------------------------
    if (!InitGameWorld()) {
        FreeModel(largeRoomID);
        FreeModel(smallRoomID);
        FreeModel(playerModelID);
        FreeShaders(shaderProgram);

        CloseWindow(&appWindow);
        return 1;
    }

    // ------------------------------------
    // ゲーム状態の初期化
    // ------------------------------------
    GameInitData game;
    InitGameState(&game, shaderProgram);

int networkConnected = 0;

if (Network_Connect("127.0.0.1", 50000) < 0) {
    printf("サーバーへの接続に失敗しました。\n");
} else {
    networkConnected = 1;

    printf("サーバーへの接続に成功しました。プレイヤーID：%d\n",
           Network_GetPlayerID());
}

    // 既存のゲームループからゲーム状態にアクセスするための別名
    #define playerX game.playerX
    #define playerY game.playerY
    #define playerZ game.playerZ
    #define playerScale game.playerScale
    #define playerDirection game.playerDirection
    #define playerTargetDirection game.playerTargetDirection
    #define playerRotationSpeed game.playerRotationSpeed

    #define view game.view
    #define projection game.projection
    #define cameraMode game.cameraMode
    #define cameraYaw game.cameraYaw
    #define cameraPitch game.cameraPitch

    #define mapMode game.mapMode
    #define selectedNPCID game.selectedNPCID
    #define previousSelectedNPCID game.previousSelectedNPCID

    // メインループ
    // ------------------------------------
    int running = 1;


    Uint32 previousTime = SDL_GetTicks();
    Uint32 lastNetworkSend = SDL_GetTicks();

    while (running) {

        // ====================================
        // イベント処理
        // ====================================
        int spacePressed = 0;
        HandleInputEvents(&running, &mapMode, &spacePressed);

        // ====================================
        // 経過時間
        // ====================================
        Uint32 currentTime = SDL_GetTicks();

        float deltaTime =
            (currentTime - previousTime) / 1000.0f;

        previousTime = currentTime;

        // キーボード入力
        const Uint8 *keys = GetInputKeys();

        // ゲーム状態の更新
        UpdateGameFrame(
            &game,
            keys,
            deltaTime,
            currentTime,
            &lastMapChange,
            MAP_CHANGE_INTERVAL
        );

        // ====================================
// ネットワーク通信
// ====================================
if (networkConnected) {

    // 100ミリ秒ごとに自分の位置を送信
    if (currentTime - lastNetworkSend >= 100) {

        char sendBuffer[160];

        snprintf(
            sendBuffer,
            sizeof(sendBuffer),
            "STATE,%d,%.3f,%.3f,%.3f,%.3f\n",
            Network_GetPlayerID(),
            playerX,
            playerY,
            playerZ,
            playerDirection
        );

        if (Network_Send(sendBuffer) < 0) {
            printf("位置情報の送信に失敗しました。\n");
            networkConnected = 0;
        }

        lastNetworkSend = currentTime;
    }

    // 他プレイヤーの位置情報を受信
    if (networkConnected) {
        char receiveBuffer[256];
        int received;

        while ((received = Network_Receive(
            receiveBuffer,
            sizeof(receiveBuffer)
        )) > 0) {

            int senderID;
            int dataPlayerID;

            float x, y, z, direction;

            // サーバーから届く形式：
            // Player 2: STATE,2,1.000,0.000,2.000,90.000

            if (sscanf(
                receiveBuffer,
                "Player %d: STATE,%d,%f,%f,%f,%f",
                &senderID,
                &dataPlayerID,
                &x,
                &y,
                &z,
                &direction
            ) == 6) {

                // 自分以外のプレイヤーだけ更新
                if (senderID != Network_GetPlayerID()
                    && senderID == dataPlayerID
                    && senderID >= 1
                    && senderID <= 4) {

                    SetRemotePlayerState(
                        senderID,
                        x,
                        y,
                        z,
                        direction
                    );
                }
            }
        }

        if (received < 0) {
            printf("サーバーとの通信が終了しました。\n");
            networkConnected = 0;
        }
    }
}

        // Spaceキーで選択中のNPCにアクション
// 行動不能中はアクションできない
if (spacePressed && game.immobilizedTimer <= 0.0f) {
    HandleNPCAction(&game);
}

        // シェーダー設定
        SetShaderMatrices(
            shaderProgram,
            view,
            projection
        );

        // ====================================
// ゲーム画面の描画
        DrawGameScene(
            largeRoomID,
            smallRoomID,
            playerModelID,
            shaderProgram,
            cameraMode,
            selectedNPCID,
            playerX,
            playerY,
            playerZ,
            playerScale,
            playerDirection
        );

                // 他プレイヤーの描画
        DrawRemotePlayers(
            playerModelID,
            shaderProgram,
            playerScale
        );

        // 盲目状態の画面暗転
        DrawBlindnessOverlay(game.blindnessTimer);

        // UI描画
        DrawGameUI(
            cameraMode,
            selectedNPCID,
            mapMode,
            playerX,
            playerZ,
            cameraYaw
        );

        // ====================================
        // 画面更新
        // ====================================
        SDL_GL_SwapWindow(window);
    }

Network_Close();

    // ========================================
    // 終了処理
    // ========================================

    CleanupGame(
        largeRoomID,
        smallRoomID,
        playerModelID,
        shaderProgram,
        &appWindow
    );

    return 0;
}
