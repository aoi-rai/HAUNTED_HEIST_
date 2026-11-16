/* ui1.c 

スタート画面は
・操作説明ボタン（ボタンを押すと操作説明の書かれた画像（今は黄色画面でいい）が出るようにする）
・ストーリーボタン（ボタンを押すとストーリーの書かれた画像（今は青色画面でいい）が出るようにする）
・次へボタン（ボタンを押すと他プレイヤー待機画面に行くようにする）
・ゲーム終了ボタン（ゲーム終了してウィンドウ閉じる）
を表示する

他プレイヤー待機画面は
２人以上集まったらスタートボタンが押せるようにする（赤四角が自分、青四角、黄四角、緑四角の順に待機プレイヤーが表示されるようにする）

終了画面は
・勝利判定を表示する
・リトライボタン
・ゲーム終了ボタン（ゲーム終了してウィンドウ閉じる）
を表示する

＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝

メインループ側（SDL_Renderer を使う場合の呼び出し例）

SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

// ... メインループ内 ...
SDL_RenderClear(renderer);

if (currentScene == SCENE_TITLE) {
    DrawTitleScene(renderer, 1100, 720, selectedMenu);
} else if (currentScene == SCENE_WAITING) {
    DrawWaitingScene(renderer, 1100, 720, connectedPlayers, 4);
} else if (currentScene == SCENE_RESULT) {
    DrawResultScene(renderer, 1100, 720, winnerId, myPlayerId);
}

SDL_RenderPresent(renderer);

*/
#include "ui1.h"
#include <stdio.h>

int InitUI(void)
{
    printf("SDL UIシステム初期化完了\n");
    return 1;
}

void CloseUI(void)
{
    // 解放処理が必要な場合はここに追加
}

/* 1. スタート画面の描画・操作 */
void DrawTitleScene(SDL_Renderer *renderer, int windowWidth, int windowHeight, int selectedMenu, GameScene *currentScene, int *running)
{
    // 背景（濃い紺色）
    SDL_SetRenderDrawColor(renderer, 15, 20, 30, 255);
    SDL_Rect bgRect = {0, 0, windowWidth, windowHeight};
    SDL_RenderFillRect(renderer, &bgRect);

    // タイトルロゴ枠
    SDL_SetRenderDrawColor(renderer, 38, 51, 76, 255);
    SDL_Rect titleRect = {(windowWidth - 500) / 2, 80, 500, 100};
    SDL_RenderFillRect(renderer, &titleRect);

    // 4つのボタン（操作説明、ストーリー、次へ、ゲーム終了）
    int btnW = 360;
    int btnH = 50;
    int btnX = (windowWidth - btnW) / 2;
    int startY = 230;
    int gap = 70;

    for (int i = 0; i < 4; i++) {
        if (selectedMenu == i) {
            SDL_SetRenderDrawColor(renderer, 217, 166, 51, 255); // 選択中（オレンジ）
        } else {
            SDL_SetRenderDrawColor(renderer, 64, 64, 76, 255);   // 非選択
        }
        SDL_Rect btnRect = {btnX, startY + i * gap, btnW, btnH};
        SDL_RenderFillRect(renderer, &btnRect);
    }
}

/* 操作説明画面（黄色背景） */
void DrawHowToScene(SDL_Renderer *renderer, int windowWidth, int windowHeight, GameScene *currentScene)
{
    (void)currentScene;
    // 黄色画面
    SDL_SetRenderDrawColor(renderer, 240, 220, 80, 255);
    SDL_Rect bgRect = {0, 0, windowWidth, windowHeight};
    SDL_RenderFillRect(renderer, &bgRect);
}

/* ストーリー画面（青色背景） */
void DrawStoryScene(SDL_Renderer *renderer, int windowWidth, int windowHeight, GameScene *currentScene)
{
    (void)currentScene;
    // 青色画面
    SDL_SetRenderDrawColor(renderer, 70, 130, 220, 255);
    SDL_Rect bgRect = {0, 0, windowWidth, windowHeight};
    SDL_RenderFillRect(renderer, &bgRect);
}

/* 2. 他のプレイヤー接続待機画面 */
void DrawWaitingScene(SDL_Renderer *renderer, int windowWidth, int windowHeight, int connectedPlayers, int maxPlayers, int selectedMenu, GameScene *currentScene, int *running)
{
    (void)currentScene;
    (void)running;

    // 背景（ダークグリーン系）
    SDL_SetRenderDrawColor(renderer, 13, 26, 26, 255);
    SDL_Rect bgRect = {0, 0, windowWidth, windowHeight};
    SDL_RenderFillRect(renderer, &bgRect);

    // 4人分のスロット（自分=赤、青、黄、緑の順）
    int slotW = 160;
    int slotH = 200;
    int gap = 25;
    int totalW = slotW * maxPlayers + gap * (maxPlayers - 1);
    int startX = (windowWidth - totalW) / 2;
    int startY = 220;

    for (int i = 0; i < maxPlayers; i++) {
        SDL_Rect slotRect = {startX + i * (slotW + gap), startY, slotW, slotH};
        
        if (i < connectedPlayers) {
            // 参加順の色分け：0=自分(赤), 1=青, 2=黄, 3=緑
            if (i == 0)      SDL_SetRenderDrawColor(renderer, 220, 60, 60, 255);   // 赤（自分）
            else if (i == 1) SDL_SetRenderDrawColor(renderer, 60, 120, 220, 255); // 青
            else if (i == 2) SDL_SetRenderDrawColor(renderer, 220, 200, 60, 255); // 黄
            else             SDL_SetRenderDrawColor(renderer, 60, 180, 90, 255);  // 緑
        } else {
            // 未接続（ダークグレー）
            SDL_SetRenderDrawColor(renderer, 46, 51, 56, 255);
        }
        SDL_RenderFillRect(renderer, &slotRect);
    }

    // スタートボタン（2人以上集まったら押せる）
    int btnW = 300;
    int btnH = 60;
    int btnX = (windowWidth - btnW) / 2;
    int btnY = 520;

    if (connectedPlayers >= 2) {
        if (selectedMenu == 0) {
            SDL_SetRenderDrawColor(renderer, 217, 166, 51, 255); // 選択中ハイライト
        } else {
            SDL_SetRenderDrawColor(renderer, 51, 178, 115, 255); // アクティブ（緑）
        }
    } else {
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);       // 無効（グレー）
    }
    SDL_Rect startBtnRect = {btnX, btnY, btnW, btnH};
    SDL_RenderFillRect(renderer, &startBtnRect);
}

/* 3. 終了画面（リザルト）の描画 */
void DrawResultScene(SDL_Renderer *renderer, int windowWidth, int windowHeight, int winnerId, int myPlayerId, int selectedMenu, GameScene *currentScene, int *running)
{
    (void)currentScene;
    (void)running;

    // 勝敗によって背景色を変更
    if (winnerId == myPlayerId) {
        SDL_SetRenderDrawColor(renderer, 46, 38, 13, 255); // 勝利（ゴールド・茶系）
    } else {
        SDL_SetRenderDrawColor(renderer, 31, 13, 20, 255); // 敗北（ダークレッド系）
    }
    SDL_Rect bgRect = {0, 0, windowWidth, windowHeight};
    SDL_RenderFillRect(renderer, &bgRect);

    // 結果表示パネル
    SDL_SetRenderDrawColor(renderer, 51, 56, 71, 255);
    SDL_Rect panelRect = {(windowWidth - 600) / 2, 140, 600, 260};
    SDL_RenderFillRect(renderer, &panelRect);

    // 2つのボタン（リトライ、ゲーム終了）
    int btnW = 280;
    int btnH = 55;
    int btnX = (windowWidth - btnW) / 2;

    // リトライボタン
    if (selectedMenu == 0) {
        SDL_SetRenderDrawColor(renderer, 217, 166, 51, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 70, 80, 100, 255);
    }
    SDL_Rect retryRect = {btnX, 440, btnW, btnH};
    SDL_RenderFillRect(renderer, &retryRect);

    // ゲーム終了ボタン
    if (selectedMenu == 1) {
        SDL_SetRenderDrawColor(renderer, 217, 166, 51, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 70, 80, 100, 255);
    }
    SDL_Rect quitRect = {btnX, 520, btnW, btnH};
    SDL_RenderFillRect(renderer, &quitRect);
}
/* end of ui1.c */