#ifndef UI2_H
#define UI2_H

int InitUI(void);
void CleanupUI(void);

void DrawText2D(const char *text, int x, int y, int size);
void DrawTextCentered(const char *text, int y, int size);
void DrawRect2D(int x, int y, int width, int height);
void DrawPlatform2D(int centerX, int y, int width, int height, float r, float g, float b);

void DrawGameUI(
    int cameraMode,
    int selectedNPCID,
    int mapMode,
    float playerX,
    float playerZ,
    float cameraYaw
);

#endif
