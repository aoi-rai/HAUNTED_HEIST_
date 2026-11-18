#include "ui2.h"

#include "camera2.h"
#include "map_display2.h"

void DrawGameUI(
    int cameraMode,
    int selectedNPCID,
    int mapMode,
    float playerX,
    float playerZ,
    float cameraYaw
)
{
    // 一人称カメラのとき照準を表示
    if (cameraMode == CAMERA_PLAYER) {
        DrawReticle(selectedNPCID >= 0);
    }

    // マップを描画
    DrawMap(
        mapMode,
        playerX,
        playerZ,
        cameraYaw
    );
}
