#ifndef CAMERA2_H
#define CAMERA2_H

#define CAMERA_EDITOR 0
#define CAMERA_PLAYER 1
#define CAMERA_GUIDE 2
#define CAMERA_WAITING 3

void CreatePerspective(
    float matrix[16],
    float fov,
    float aspect,
    float nearPlane,
    float farPlane
);

void Update3DCamera(
    float view[16],
    int cameraMode,
    float playerX,
    float playerY,
    float playerZ,
    float cameraYaw,
    float cameraPitch
);


void DrawReticle(int selected);

#endif
