
#include <stdio.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

#define CAMERA_EDITOR 0
#define CAMERA_PLAYER 1

#define PI 3.14159265f

#define SCREEN_TITLE     0
#define SCREEN_GUIDE     1
#define SCREEN_CONTROLS  2
#define SCREEN_CONCEPT   3
#define SCREEN_GAME      4
#define SCREEN_WAITING   5
#define SCREEN_COUNTDOWN 6

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    srand((unsigned int)time(NULL));

    /* ウィンドウ初期化 */
    WindowContext appWindow;
    WindowContext waitingWindow = {0};

    if (!InitWindow(&appWindow)) {
        return 1;
    }

    SDL_Window *window = appWindow.window;

    /* シェーダー読み込み */
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

    /* MAP変更タイマー */
    Uint32 lastMapChange = SDL_GetTicks();
    const Uint32 MAP_CHANGE_INTERVAL = 30000;

    /* モデル読み込み */
    int largeRoomID = 0;
    int smallRoomID = 0;
    int playerModelID = 0;
    int waitingPlayerModelID = 0;

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

    /* 待機画面でもゲーム中と同じプレイヤーモデルを使用 */
    waitingPlayerModelID = playerModelID;

    /* ゲームワールド初期化 */
    if (!InitGameWorld()) {
        FreeModel(largeRoomID);
        FreeModel(smallRoomID);
        FreeModel(playerModelID);
        FreeModel(waitingPlayerModelID);
        FreeShaders(shaderProgram);
        CloseWindow(&appWindow);
        return 1;
    }

    /* ゲーム状態初期化 */
    GameInitData game;
    InitGameState(&game, shaderProgram);

    InitRemotePlayers();

    /* UI初期化 */
    if (!InitUI()) {
        printf("UIの初期化に失敗しました。\n");

        CleanupGame(
            largeRoomID,
            smallRoomID,
            playerModelID,
            waitingPlayerModelID,
            shaderProgram,
            &appWindow
        );

        return 1;
    }

    /* ネットワーク状態 */
    int networkConnected = 0;

    /* 待機画面の状態 */
    int waitingPlayerCount = 0;
    int waitingReady[4] = {0, 0, 0, 0};
    int waitingCanStart = 0;
    int startRequestSent = 0;
    int myReady = 0;

    Uint32 countdownStartTime = 0;

    /* 現在のホストID */
    int waitingHostPlayerID = 0;

    /* ゲーム状態への別名 */
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

    /* メインループ */
    int running = 1;
    int screenState = SCREEN_TITLE;

    /* ガイド画面の選択位置 */
    int guideSelection = 0;

    /* ガイド画面専用カメラの回転角度 */
    float guideCameraAngle = 0.58f;

    Uint32 previousTime = SDL_GetTicks();
    Uint32 lastNetworkSend = SDL_GetTicks();

    while (running) {

        /* イベント処理 */
        int spacePressed = 0;

        HandleInputEvents(
            &running,
            &mapMode,
            &spacePressed
        );

        /* 経過時間 */
        Uint32 currentTime = SDL_GetTicks();

        float deltaTime =
            (currentTime - previousTime) / 1000.0f;

        previousTime = currentTime;

        const Uint8 *keys = GetInputKeys();

        /* ====================================
           タイトル画面
           ==================================== */
        if (screenState == SCREEN_TITLE) {

            if (WasKeyPressed(SDLK_RETURN) ||
                WasKeyPressed(SDLK_KP_ENTER)) {
                screenState = SCREEN_GUIDE;
            }

            glViewport(0, 0, 1000, 720);
            glClear(
                GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT
            );

            DrawTextCentered("HAUNTED HEIST", 250, 64);
            DrawTextCentered("PRESS ENTER", 430, 28);

            SDL_GL_SwapWindow(window);
            continue;
        }

        /* ====================================
           ガイド画面
           ==================================== */
        if (screenState == SCREEN_GUIDE) {

            if (WasKeyPressed(SDLK_LEFT)) {
                guideSelection--;

                if (guideSelection < 0) {
                    guideSelection = 2;
                }
            }

            if (WasKeyPressed(SDLK_RIGHT)) {
                guideSelection++;

                if (guideSelection > 2) {
                    guideSelection = 0;
                }
            }

            if (WasKeyPressed(SDLK_RETURN) ||
                WasKeyPressed(SDLK_KP_ENTER)) {

                if (guideSelection == 0) {
                    screenState = SCREEN_CONTROLS;
                }
                else if (guideSelection == 1) {
                    screenState = SCREEN_CONCEPT;
                }
                else if (guideSelection == 2) {

                    if (InitSharedWindow(
                        &appWindow,
                        &waitingWindow
                    )) {
                        SDL_HideWindow(window);
                        screenState = SCREEN_WAITING;

                        if (!networkConnected) {

                            if (Network_Connect(
                                "127.0.0.1",
                                50000
                            ) < 0) {
                                printf(
                                    "サーバーへの接続に失敗しました。\n"
                                );
                            }
                            else {
                                networkConnected = 1;

                                myReady = 0;
                                waitingPlayerCount = 0;
                                waitingCanStart = 0;
                                waitingHostPlayerID = 0;
                                startRequestSent = 0;

                                printf(
                                    "サーバーへの接続に成功しました。"
                                    "プレイヤーID：%d\n",
                                    Network_GetPlayerID()
                                );
                            }
                        }
                    }
                    else {
                        printf(
                            "待機用ウィンドウの作成に失敗しました。\n"
                        );
                    }
                }
            }

            // ガイド画面でもNPCを移動させる
            UpdateNPCs(deltaTime);

            // ガイド画面専用カメラをゆっくり回転
            guideCameraAngle += deltaTime * 0.09f;

            if (guideCameraAngle >= 6.2831853f) {
                guideCameraAngle -= 6.2831853f;
            }

            Update3DCamera(
                view,
                CAMERA_GUIDE,
                0.0f,
                0.0f,
                0.0f,
                guideCameraAngle,
                0.0f
            );

            SetShaderMatrices(
                shaderProgram,
                view,
                projection
            );

            DrawGameScene(
                largeRoomID,
                smallRoomID,
                playerModelID,
                shaderProgram,
                CAMERA_EDITOR,
                -1,
                playerX,
                playerY,
                playerZ,
                playerScale,
                playerDirection
            );

            DrawTextCentered("GUIDE", 100, 48);

            DrawText2D(
                guideSelection == 0
                    ? "> 操作説明 <"
                    : "  操作説明  ",
                180, 400, 30
            );

            DrawText2D(
                guideSelection == 1
                    ? "> コンセプト <"
                    : "  コンセプト  ",
                430, 400, 30
            );

            DrawText2D(
                guideSelection == 2
                    ? "> 次へ <"
                    : "  次へ  ",
                720, 400, 30
            );

            DrawTextCentered(
                "LEFT / RIGHT : SELECT",
                550, 20
            );

            DrawTextCentered(
                "ENTER : DECIDE",
                590, 20
            );

            SDL_GL_SwapWindow(window);
            continue;
        }

        /* ====================================
           待機画面
           ==================================== */
        if (screenState == SCREEN_WAITING) {

            /* ESC：自分だけ退出 */
            if (WasKeyPressedInWindow(
        SDLK_ESCAPE,
        SDL_GetWindowID(waitingWindow.window))) {

             printf(
    "[WAIT EXIT] reason=ESC PID=%d windowID=%u\n",
    getpid(),
    SDL_GetWindowID(waitingWindow.window)
);

                if (networkConnected) {
                    Network_Close();
                    networkConnected = 0;
                }

                myReady = 0;
                waitingPlayerCount = 0;
                waitingCanStart = 0;
                waitingHostPlayerID = 0;
                startRequestSent = 0;

                for (int i = 0; i < 4; i++) {
                    waitingReady[i] = 0;
                }

                CloseSharedWindow(
                    &appWindow,
                    &waitingWindow
                );

                SDL_ShowWindow(window);

                SDL_GL_MakeCurrent(
                    appWindow.window,
                    appWindow.context
                );

                screenState = SCREEN_GUIDE;
                continue;
            }

            /* R：再接続 */
            if (!networkConnected &&
                WasKeyPressed(SDLK_r)) {

                if (Network_Connect(
                    "127.0.0.1",
                    50000
                ) < 0) {
                    printf(
                        "サーバーへの再接続に失敗しました。\n"
                    );
                }
                else {
                    networkConnected = 1;

                    myReady = 0;
                    waitingPlayerCount = 0;
                    waitingCanStart = 0;
                    waitingHostPlayerID = 0;
                    startRequestSent = 0;

                    for (int i = 0; i < 4; i++) {
                        waitingReady[i] = 0;
                    }

                    printf(
                        "サーバーへの再接続に成功しました。"
                        "プレイヤーID：%d\n",
                        Network_GetPlayerID()
                    );
                }
            }


            /* サーバーから待機状態を受信 */
            if (networkConnected) {

                char receiveBuffer[256];
                int received;

                while ((received = Network_Receive(
                    receiveBuffer,
                    sizeof(receiveBuffer)
                )) > 0) {

                    printf(
                        "[CLIENT RECEIVE] %s\n",
                        receiveBuffer
                    );

                    /* START通知 */
                    if (strncmp(
                        receiveBuffer,
                        "START",
                        5
                    ) == 0) {

                        countdownStartTime = SDL_GetTicks();

                        CloseSharedWindow(
                            &appWindow,
                            &waitingWindow
                        );

                        SDL_ShowWindow(window);

                        SDL_GL_MakeCurrent(
                            appWindow.window,
                            appWindow.context
                        );

                        screenState = SCREEN_COUNTDOWN;

                        printf(
                            "ゲーム開始通知を受信しました。\n"
                        );

                        break;
                    }

                    /* STATUS受信 */
                    int count;
                    int ready1, ready2, ready3, ready4;
                    int canStart;
                    int hostID;

                    if (sscanf(
                        receiveBuffer,
                        "STATUS,%d,%d,%d,%d,%d,%d,%d",
                        &count,
                        &ready1,
                        &ready2,
                        &ready3,
                        &ready4,
                        &canStart,
                        &hostID
                    ) == 7) {

                        waitingPlayerCount = count;

                        waitingReady[0] = ready1;
                        waitingReady[1] = ready2;
                        waitingReady[2] = ready3;
                        waitingReady[3] = ready4;

                        waitingCanStart = canStart;

                        /* ホスト交代を反映 */
                        if (hostID >= 1 && hostID <= 4) {
                            waitingHostPlayerID = hostID;
                        }
                        else {
                            waitingHostPlayerID = 0;
                        }

                        int myID = Network_GetPlayerID();

                        if (myID >= 1 && myID <= 4) {
                            myReady = waitingReady[myID - 1];
                        }
                    }
                }

                /* 通信切断 */
                if (received < 0) {

                    printf(
                        "サーバーとの通信が終了しました。\n"
                    );

                    networkConnected = 0;
                    myReady = 0;
                    waitingPlayerCount = 0;
                    waitingCanStart = 0;
                    waitingHostPlayerID = 0;
                    startRequestSent = 0;
                }
            }

            if (screenState == SCREEN_COUNTDOWN) {
                continue;
            }

            /* R：準備完了・解除 */
            if (networkConnected &&
                WasKeyPressed(SDLK_r)) {

                if (myReady) {

                    if (Network_Send("UNREADY\n") < 0) {
                        networkConnected = 0;
                        myReady = 0;
                        waitingHostPlayerID = 0;
                    }
                }
                else {

                    if (Network_Send("READY\n") < 0) {
                        networkConnected = 0;
                        myReady = 0;
                        waitingHostPlayerID = 0;
                    }
                }
            }

            /* 現在のホストだけがゲーム開始を要求 */
            if (networkConnected &&
                waitingCanStart &&
                !startRequestSent &&
                waitingHostPlayerID >= 1 &&
                waitingHostPlayerID <= 4 &&
                Network_GetPlayerID() == waitingHostPlayerID &&
                (WasKeyPressed(SDLK_RETURN) ||
                 WasKeyPressed(SDLK_KP_ENTER))) {

                if (Network_Send("START\n") < 0) {

                    printf(
                        "ゲーム開始要求の送信に失敗しました。\n"
                    );

                    networkConnected = 0;
                    waitingHostPlayerID = 0;
                }
                else {
                    startRequestSent = 1;

                    printf(
                        "ゲーム開始要求を送信しました。\n"
                    );
                }
            }

            /* 待機用ウィンドウへ切り替え */
            if (SDL_GL_MakeCurrent(
                waitingWindow.window,
                waitingWindow.context
            ) < 0) {

                printf(
                    "待機用ウィンドウの描画切り替えに失敗しました: %s\n",
                    SDL_GetError()
                );

                CloseSharedWindow(
                    &appWindow,
                    &waitingWindow
                );

                SDL_ShowWindow(window);

                screenState = SCREEN_GUIDE;
                continue;
            }

            /* 待機画面専用カメラ */
            Update3DCamera(
                view,
                CAMERA_WAITING,
                0.0f,
                0.0f,
                0.0f,
                0.0f,
                0.0f
            );

            SetShaderMatrices(
                shaderProgram,
                view,
                projection
            );

            /* 森の背景 */
            DrawWaitingBackground(shaderProgram);

            /* プレイヤーを門の前に左から横一列で表示 */
{
    const float centerX = 0.0f;
    const float centerZ = -65.0f;

    /* 参加人数分のおばけを左から順番に表示 */
    int displayCount = waitingPlayerCount;

    if (displayCount > 4) displayCount = 4;
    if (displayCount < 0) displayCount = 0;

    for (int i = 0; i < displayCount; i++) {

        /* 左端から順番に配置 */
        float offset = 12.6f - i * 8.4f;

        DrawModelAtRotated(
            waitingPlayerModelID,
            shaderProgram,
            centerX + offset,
            0.0f,
            centerZ,
            0.020f,
            90.0f
        );
    }
}

            /* 待機画面UI */
            DrawTextCentered("WAITING ROOM", 35, 42);

            if (networkConnected) {
                char text[128];

                snprintf(
                    text,
                    sizeof(text),
                    "参加人数 : %d / 4",
                    waitingPlayerCount
                );
                DrawTextCentered(text, 95, 24);

                /* 各プレイヤーの名前・準備状態・枠 */
                {
                    const int centers[4] = {125, 375, 625, 875};

                    const float platformColors[4][3] = {
                        {0.05f, 0.55f, 1.00f},  /* 青 */
                        {0.65f, 0.10f, 1.00f},  /* 紫 */
                        {0.25f, 1.00f, 0.05f},  /* 緑 */
                        {0.00f, 0.85f, 1.00f}   /* 水色 */
                    };

                    for (int i = 0; i < 4; i++) {
                        int x = centers[i];

                        if (waitingReady[i]) {
                            snprintf(
                                text,
                                sizeof(text),
                                "PLAYER %d : READY",
                                i + 1
                            );
                        } else {
                            snprintf(
                                text,
                                sizeof(text),
                                "PLAYER %d : WAITING",
                                i + 1
                            );
                        }

                        /* おばけの上に表示 */
                        DrawText2D(text, x - 85, 175, 20);

                        /* プレイヤーごとの色付きステージ台 */
                        DrawPlatform2D(
                            x, 485, 150, 35,
                            platformColors[i][0],
                            platformColors[i][1],
                            platformColors[i][2]
                        );
                    }
                }

                if (myReady) {
                    DrawTextCentered(
                        "あなたは準備完了です！",
                        580, 22
                    );
                } else {
                    DrawTextCentered(
                        "R : 準備完了にする",
                        580, 22
                    );
                }

                if (waitingCanStart) {
                    DrawTextCentered("全員準備完了！", 615, 22);

                    if (waitingHostPlayerID >= 1 &&
                        Network_GetPlayerID() == waitingHostPlayerID) {
                        DrawTextCentered("ENTER : ゲーム開始", 645, 20);
                    } else {
                        snprintf(
                            text,
                            sizeof(text),
                            "PLAYER %d の開始を待っています",
                            waitingHostPlayerID
                        );
                        DrawTextCentered(text, 645, 20);
                    }
                } else {
                    DrawTextCentered(
                        "全員の準備を待っています...",
                        615, 20
                    );
                }
            } else {
                DrawTextCentered(
                    "サーバーに接続できませんでした",
                    300, 28
                );
                DrawTextCentered("R : 再接続", 380, 26);
            }

            DrawTextCentered("ESC : ガイドに戻る", 685, 20);

            SDL_GL_SwapWindow(waitingWindow.window);
            continue;
        }

        /* ====================================
           操作説明画面
           ==================================== */
        if (screenState == SCREEN_CONTROLS) {

            glViewport(0, 0, 1000, 720);

            glClear(
                GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT
            );

            DrawTextCentered("操作説明", 100, 48);

            DrawTextCentered("W : 前進", 230, 28);
            DrawTextCentered("S : 後退", 290, 28);
            DrawTextCentered("A : 左移動", 350, 28);
            DrawTextCentered("D : 右移動", 410, 28);
            DrawTextCentered(
                "SPACE : お化けにアクション",
                470, 28
            );
            DrawTextCentered("X : マップ切り替え", 530, 28);

            DrawTextCentered(
                "ENTER : ガイドに戻る",
                640, 22
            );

            SDL_GL_SwapWindow(window);

            if (WasKeyPressed(SDLK_RETURN) ||
                WasKeyPressed(SDLK_KP_ENTER)) {
                screenState = SCREEN_GUIDE;
            }

            continue;
        }

        /* ====================================
           コンセプト画面
           ==================================== */
        if (screenState == SCREEN_CONCEPT) {

            glViewport(0, 0, 1000, 720);

            glClear(
                GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT
            );

            DrawTextCentered("CONCEPT", 100, 48);

            DrawTextCentered(
                "お化け屋敷に隠された宝物を探し出せ！",
                250, 28
            );

            DrawTextCentered(
                "プレイヤーはNPCのお化けに紛れて行動する。",
                330, 24
            );

            DrawTextCentered(
                "怪しいお化けを見つけて、シーツを剥がそう！",
                390, 24
            );

            DrawTextCentered(
                "宝物を手に入れたら、出口を目指そう！",
                450, 24
            );

            DrawTextCentered(
                "ENTER : ガイドに戻る",
                640, 22
            );

            SDL_GL_SwapWindow(window);

            if (WasKeyPressed(SDLK_RETURN) ||
                WasKeyPressed(SDLK_KP_ENTER)) {
                screenState = SCREEN_GUIDE;
            }

            continue;
        }

        /* ====================================
           ゲーム開始カウントダウン
           ==================================== */
        if (screenState == SCREEN_COUNTDOWN) {

            Uint32 elapsed =
                SDL_GetTicks() - countdownStartTime;

            if (elapsed >= 4000) {

                lastMapChange = SDL_GetTicks();

                screenState = SCREEN_GAME;
                previousTime = currentTime;

                continue;
            }

            SetShaderMatrices(
                shaderProgram,
                view,
                projection
            );

            DrawGameScene(
                largeRoomID,
                smallRoomID,
                playerModelID,
                shaderProgram,
                CAMERA_EDITOR,
                -1,
                playerX,
                playerY,
                playerZ,
                playerScale,
                playerDirection
            );

            DrawRemotePlayers(
                playerModelID,
                shaderProgram,
                playerScale
            );

            if (elapsed < 1000) {
                DrawTextCentered("3", 330, 110);
            }
            else if (elapsed < 2000) {
                DrawTextCentered("2", 330, 110);
            }
            else if (elapsed < 3000) {
                DrawTextCentered("1", 330, 110);
            }
            else {
                DrawTextCentered("START!", 330, 80);
            }

            SDL_GL_SwapWindow(window);
            continue;
        }

        /* ====================================
           ここから既存のゲーム処理
           ==================================== */

        UpdateGameFrame(
            &game,
            keys,
            deltaTime
        );

        /* ====================================
           ネットワーク通信
           ==================================== */
        if (networkConnected) {

            /* 現在のホストがMAPを送信 */
            if (screenState == SCREEN_GAME &&
                waitingHostPlayerID >= 1 &&
                waitingHostPlayerID <= 4 &&
                Network_GetPlayerID() == waitingHostPlayerID &&
                currentTime - lastMapChange >= MAP_CHANGE_INTERVAL) {

                float positions[ROOM_COUNT][2] = {{0}};
                char mapSendBuffer[512];

                GenerateMapLayout(positions);

                int offset = snprintf(
                    mapSendBuffer,
                    sizeof(mapSendBuffer),
                    "MAP"
                );

                int valid = (
                    offset >= 0 &&
                    offset < (int)sizeof(mapSendBuffer)
                );

                for (int i = 1;
                     i < ROOM_COUNT && valid;
                     i++) {

                    int written = snprintf(
                        mapSendBuffer + offset,
                        sizeof(mapSendBuffer) - offset,
                        ",%.1f,%.1f",
                        positions[i][0],
                        positions[i][1]
                    );

                    if (written < 0 ||
                        written >=
                        (int)(sizeof(mapSendBuffer) - offset)) {
                        valid = 0;
                    }
                    else {
                        offset += written;
                    }
                }

                if (valid) {

                    int written = snprintf(
                        mapSendBuffer + offset,
                        sizeof(mapSendBuffer) - offset,
                        "\n"
                    );

                    if (written < 0 ||
                        written >=
                        (int)(sizeof(mapSendBuffer) - offset)) {
                        valid = 0;
                    }
                }

                if (valid) {

                    if (Network_Send(mapSendBuffer) < 0) {

                        printf(
                            "MAP情報の送信に失敗しました。\n"
                        );

                        networkConnected = 0;
                        waitingHostPlayerID = 0;
                    }
                    else {

                        printf(
                            "新しいMAP配置をサーバーへ送信しました。\n"
                        );

                        lastMapChange = currentTime;
                    }
                }
                else {
                    printf(
                        "MAP送信データの作成に失敗しました。\n"
                    );
                }
            }

            /* 100ミリ秒ごとに位置情報を送信 */
            if (networkConnected &&
                currentTime - lastNetworkSend >= 100) {

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

                    printf(
                        "位置情報の送信に失敗しました。\n"
                    );

                    networkConnected = 0;
                    waitingHostPlayerID = 0;
                }

                lastNetworkSend = currentTime;
            }

            /* 他プレイヤーの位置情報を受信 */
            if (networkConnected) {

                char receiveBuffer[1024];
                int received;

                while ((received = Network_Receive(
                    receiveBuffer,
                    sizeof(receiveBuffer)
                )) > 0) {

                    int senderID;
                    int dataPlayerID;

                    float x, y, z, direction;

                    /* ホストから届いたMAP */
                    if (waitingHostPlayerID >= 1 &&
                        waitingHostPlayerID <= 4) {

                        char mapPrefix[32];

                        snprintf(
                            mapPrefix,
                            sizeof(mapPrefix),
                            "Player %d: MAP,",
                            waitingHostPlayerID
                        );

                        if (strncmp(
                            receiveBuffer,
                            mapPrefix,
                            strlen(mapPrefix)
                        ) == 0) {

                            const char *cursor =
                                receiveBuffer + strlen(mapPrefix);

                            float positions[ROOM_COUNT][2] = {{0}};
                            int valid = 1;

                            for (int roomID = 1;
                                 roomID < ROOM_COUNT && valid;
                                 roomID++) {

                                char *end;

                                positions[roomID][0] =
                                    strtof(cursor, &end);

                                if (end == cursor || *end != ',') {
                                    valid = 0;
                                    break;
                                }

                                cursor = end + 1;

                                positions[roomID][1] =
                                    strtof(cursor, &end);

                                if (end == cursor) {
                                    valid = 0;
                                    break;
                                }

                                cursor = end;

                                if (roomID < ROOM_COUNT - 1) {

                                    if (*cursor != ',') {
                                        valid = 0;
                                        break;
                                    }

                                    cursor++;
                                }
                            }

                            while (*cursor == '\n' ||
                                   *cursor == '\r') {
                                cursor++;
                            }

                            if (valid && *cursor == '\0') {

                                printf(
                                    "ホストからMAP配置を受信しました。\n"
                                );

                                ApplyMapLayout(
                                    positions,
                                    currentTime,
                                    &lastMapChange,
                                    &playerX,
                                    &playerZ
                                );
                            }
                            else {
                                printf(
                                    "MAP受信データの形式が正しくありません。\n"
                                );
                            }

                            continue;
                        }
                    }

                    /* 他プレイヤーの切断通知 */
                    int disconnectedID;

                    if (sscanf(
                        receiveBuffer,
                        "DISCONNECT,%d",
                        &disconnectedID
                    ) == 1) {

                        RemoveRemotePlayer(disconnectedID);

                        printf(
                            "プレイヤー%dが切断しました。\n",
                            disconnectedID
                        );

                        continue;
                    }

                    /*
                     * サーバーから届く形式：
                     * Player 2: STATE,2,1.000,0.000,2.000,90.000
                     */
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

                        if (senderID != Network_GetPlayerID() &&
                            senderID == dataPlayerID &&
                            senderID >= 1 &&
                            senderID <= 4) {

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

                    printf(
                        "サーバーとの通信が終了しました。\n"
                    );

                    networkConnected = 0;
                    waitingHostPlayerID = 0;
                }
            }
        }

        /* 他プレイヤーの表示位置を更新 */
        UpdateRemotePlayers(deltaTime);

        /* SpaceキーでNPCにアクション */
        if (spacePressed &&
            game.immobilizedTimer <= 0.0f) {

            HandleNPCAction(&game);
        }

        /* シェーダー設定 */
        SetShaderMatrices(
            shaderProgram,
            view,
            projection
        );

        /* ゲーム画面描画 */
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

        /* 他プレイヤー描画 */
        DrawRemotePlayers(
            playerModelID,
            shaderProgram,
            playerScale
        );

        /* 盲目状態の画面暗転 */
        DrawBlindnessOverlay(game.blindnessTimer);

        /* UI描画 */
        DrawGameUI(
            cameraMode,
            selectedNPCID,
            mapMode,
            playerX,
            playerZ,
            cameraYaw
        );

        /* 画面更新 */
        SDL_GL_SwapWindow(window);
    }

    /* 待機用ウィンドウが残っていれば閉じる */
    if (waitingWindow.window != NULL) {
        CloseSharedWindow(
            &appWindow,
            &waitingWindow
        );
    }

    /* ネットワーク終了 */
    Network_Close();

    /* UI終了 */
    CleanupUI();

    /* ゲーム終了処理 */
    CleanupGame(
        largeRoomID,
        smallRoomID,
        playerModelID,
        waitingPlayerModelID,
        shaderProgram,
        &appWindow
    );

    return 0;
}
