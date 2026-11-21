#include <stdio.h>
#include <string.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "ui2.h"
#include "camera2.h"
#include "map_display2.h"

#define FONT_PATH "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc"
#define FONT_COUNT 8
#define TEXT_CACHE_COUNT 64

typedef struct {
    int size;
    TTF_Font *font;
} FontEntry;

typedef struct {
    char text[256];
    int size;
    GLuint texture;
    int width;
    int height;
} TextEntry;

static FontEntry fonts[FONT_COUNT];
static TextEntry textCache[TEXT_CACHE_COUNT];
static int fontCount = 0;
static int textCount = 0;

int InitUI(void)
{
    if (TTF_Init() < 0) {
        printf("SDL_ttf初期化失敗: %s\n", TTF_GetError());
        return 0;
    }

    printf("UI初期化成功\n");
    return 1;
}

static TTF_Font *GetFont(int size)
{
    for (int i = 0; i < fontCount; i++) {
        if (fonts[i].size == size) {
            return fonts[i].font;
        }
    }

    if (fontCount >= FONT_COUNT) {
        return NULL;
    }

    TTF_Font *font = TTF_OpenFont(FONT_PATH, size);

    if (font == NULL) {
        printf("フォント読み込み失敗: %s\n", TTF_GetError());
        return NULL;
    }

    fonts[fontCount].size = size;
    fonts[fontCount].font = font;
    fontCount++;

    return font;
}

static TextEntry *GetTextTexture(const char *text, int size)
{
    for (int i = 0; i < textCount; i++) {
        if (textCache[i].size == size &&
            strcmp(textCache[i].text, text) == 0) {
            return &textCache[i];
        }
    }

    if (textCount >= TEXT_CACHE_COUNT) {
        return NULL;
    }

    TTF_Font *font = GetFont(size);

    if (font == NULL) {
        return NULL;
    }

    SDL_Color color = {255, 255, 255, 255};

    SDL_Surface *surface =
        TTF_RenderUTF8_Blended(font, text, color);

    if (surface == NULL) {
        printf("文字画像の作成失敗: %s\n", TTF_GetError());
        return NULL;
    }

    SDL_Surface *rgba =
        SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);

    SDL_FreeSurface(surface);

    if (rgba == NULL) {
        return NULL;
    }

    TextEntry *entry = &textCache[textCount];

    strncpy(entry->text, text, sizeof(entry->text) - 1);
    entry->text[sizeof(entry->text) - 1] = '\0';
    entry->size = size;
    entry->width = rgba->w;
    entry->height = rgba->h;

    glGenTextures(1, &entry->texture);
    glBindTexture(GL_TEXTURE_2D, entry->texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        rgba->w,
        rgba->h,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        rgba->pixels
    );

    SDL_FreeSurface(rgba);

    textCount++;
    return entry;
}

void DrawText2D(const char *text, int x, int y, int size)
{
    TextEntry *entry = GetTextTexture(text, size);

    if (entry == NULL) {
        return;
    }

    GLint oldProgram;
    GLint oldMatrixMode;
    GLint viewport[4];

    glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
    glGetIntegerv(GL_MATRIX_MODE, &oldMatrixMode);
    glGetIntegerv(GL_VIEWPORT, viewport);

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glUseProgram(0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, entry->texture);
    glColor4f(1, 1, 1, 1);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, viewport[2], viewport[3], 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    float right = (float)(x + entry->width);
    float bottom = (float)(y + entry->height);

    glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex2f((float)x, (float)y);

        glTexCoord2f(1, 0);
        glVertex2f(right, (float)y);

        glTexCoord2f(1, 1);
        glVertex2f(right, bottom);

        glTexCoord2f(0, 1);
        glVertex2f((float)x, bottom);
    glEnd();

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(oldMatrixMode);
    glUseProgram((GLuint)oldProgram);

    glPopAttrib();
}

void DrawTextCentered(const char *text, int y, int size)
{
    TextEntry *entry = GetTextTexture(text, size);

    if (entry == NULL) {
        return;
    }

    DrawText2D(text, (1000 - entry->width) / 2, y, size);
}

void DrawRect2D(int x, int y, int width, int height)
{
    GLint oldProgram;
    GLint oldMatrixMode;
    GLint viewport[4];

    glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
    glGetIntegerv(GL_MATRIX_MODE, &oldMatrixMode);
    glGetIntegerv(GL_VIEWPORT, viewport);

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, viewport[2], viewport[3], 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
    glLineWidth(2.0f);

    glBegin(GL_LINE_LOOP);
        glVertex2f((float)x, (float)y);
        glVertex2f((float)(x + width), (float)y);
        glVertex2f((float)(x + width), (float)(y + height));
        glVertex2f((float)x, (float)(y + height));
    glEnd();

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(oldMatrixMode);
    glUseProgram((GLuint)oldProgram);

    glPopAttrib();
}

void DrawPlatform2D(
    int centerX, int y, int width, int height,
    float r, float g, float b
)
{
    GLint oldProgram;
    GLint oldMatrixMode;
    GLint viewport[4];

    glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
    glGetIntegerv(GL_MATRIX_MODE, &oldMatrixMode);
    glGetIntegerv(GL_VIEWPORT, viewport);

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, viewport[2], viewport[3], 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    float halfW = width * 0.5f;
    float topHalfW = width * 0.38f;

    float leftTop = centerX - topHalfW;
    float rightTop = centerX + topHalfW;
    float leftBottom = centerX - halfW;
    float rightBottom = centerX + halfW;

    /* 台の内側 */
    glColor4f(r * 0.25f, g * 0.25f, b * 0.25f, 0.65f);

    glBegin(GL_QUADS);
        glVertex2f(leftTop, (float)y);
        glVertex2f(rightTop, (float)y);
        glVertex2f(rightBottom, (float)(y + height));
        glVertex2f(leftBottom, (float)(y + height));
    glEnd();

    /* 台の外枠 */
    glColor4f(r, g, b, 1.0f);
    glLineWidth(4.0f);

    glBegin(GL_LINE_LOOP);
        glVertex2f(leftTop, (float)y);
        glVertex2f(rightTop, (float)y);
        glVertex2f(rightBottom, (float)(y + height));
        glVertex2f(leftBottom, (float)(y + height));
    glEnd();

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(oldMatrixMode);
    glUseProgram((GLuint)oldProgram);

    glPopAttrib();
}

void CleanupUI(void)
{
    for (int i = 0; i < textCount; i++) {
        if (textCache[i].texture != 0) {
            glDeleteTextures(1, &textCache[i].texture);
        }
    }

    for (int i = 0; i < fontCount; i++) {
        TTF_CloseFont(fonts[i].font);
    }

    textCount = 0;
    fontCount = 0;

    TTF_Quit();
}

// 既存のゲームUI
void DrawGameUI(
    int cameraMode,
    int selectedNPCID,
    int mapMode,
    float playerX,
    float playerZ,
    float cameraYaw
)
{
    if (cameraMode == CAMERA_PLAYER) {
        DrawReticle(selectedNPCID >= 0);
    }

    DrawMap(mapMode, playerX, playerZ, cameraYaw);
}
