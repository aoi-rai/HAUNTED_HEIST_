#include "game_cleanup2.h"

#include "model2.h"
#include "environment2.h"
#include "shader2.h"

void CleanupGame(
    int largeRoomID,
    int smallRoomID,
    int playerModelID,
    unsigned int shaderProgram,
    WindowContext *appWindow
)
{
    // モデルの解放
    FreeModel(largeRoomID);
    FreeModel(smallRoomID);
    FreeModel(playerModelID);

    // 環境モデルの解放
    FreeEnvironment();

    // シェーダーの解放
    FreeShaders(shaderProgram);

    // ウィンドウとSDLの終了
    CloseWindow(appWindow);
}
