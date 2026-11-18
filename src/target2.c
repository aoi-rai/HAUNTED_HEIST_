#include <stdio.h>
#include "target2.h"
#include "npc2.h"
#include "camera2.h"

void UpdateTargetNPC(
    int cameraMode,
    float playerX,
    float playerY,
    float playerZ,
    float cameraYaw,
    float cameraPitch,
    int *selectedNPCID,
    int *previousSelectedNPCID
)
{
    *selectedNPCID = FindTargetNPC(
        cameraMode,
        playerX,
        playerY,
        playerZ,
        cameraYaw,
        cameraPitch
    );

    if (*selectedNPCID != *previousSelectedNPCID) {

        if (*selectedNPCID >= 0) {
            printf("選択中のNPC: %d\n", *selectedNPCID);
        } else {
            if (cameraMode == CAMERA_PLAYER) {
                printf("NPCの選択解除\n");
            }
        }

        *previousSelectedNPCID = *selectedNPCID;
    }
}
