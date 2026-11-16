/* ui1.h */
#ifndef UI1_H
#define UI1_H

#include <SDL2/SDL.h>

// ゲーム全体の画面状態
typedef enum {
    SCENE_TITLE,    // スタート画面
    SCENE_HOWTO,    // 操作説明画面
    SCENE_STORY,    // ストーリー画面
    SCENE_WAITING,  // 他のプレイヤー接続待機画面
    SCENE_PLAYING,  // ゲーム本編
    SCENE_RESULT    // 終了画面（勝敗表示）
} GameScene;

// UIシステムの初期化
int InitUI(void);

// UIシステムの終了処理
void CloseUI(void);

// 各画面の描画および入力・遷移処理
void DrawTitleScene(SDL_Renderer *renderer, int windowWidth, int windowHeight, int selectedMenu, GameScene *currentScene, int *running);
void DrawHowToScene(SDL_Renderer *renderer, int windowWidth, int windowHeight, GameScene *currentScene);
void DrawStoryScene(SDL_Renderer *renderer, int windowWidth, int windowHeight, GameScene *currentScene);
void DrawWaitingScene(SDL_Renderer *renderer, int windowWidth, int windowHeight, int connectedPlayers, int maxPlayers, int selectedMenu, GameScene *currentScene, int *running);
void DrawResultScene(SDL_Renderer *renderer, int windowWidth, int windowHeight, int winnerId, int myPlayerId, int selectedMenu, GameScene *currentScene, int *running);

#endif
/* end of ui1.h */