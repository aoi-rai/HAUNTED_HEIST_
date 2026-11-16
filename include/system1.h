/*
 *  ファイル名	: system.h
 *  機能	: 共通変数，外部関数の定義
 */

#ifndef SYSTEM1_H
#define SYSTEM1_H

#include <SDL2/SDL.h>

#define MAP_WIDTH 720
#define MAP_HEIGHT 720
#define WD_WIDTH 720
#define WD_HEIGHT 720

typedef struct {
    int playerdId;
    float positionX;
    float positionY;
    int isActive;
    float stunTime;
    int hasTreasure;
    int haskey;
} Player;

typedef struct {
    int npcId;
    float positionX;
    float positionY;
    int moveState;
    int treasureID;
} NPC;

typedef struct {
    int chestId;
    float positionX;
    float positionY;
    int chestType;
    int isOpened;
    int ownerId;
} TreasureChest;

typedef struct {
    float x, y;
    float distance;
    float height;
    float angle;
} Camera;

/* システム関数 */
void DestroySystem(void);
void UpdatePlayer(Player *player, float deltaTime);
void MovePlayer(Player *player, float inputX, float inputY, float deltaTime);
void SetPlayerStun(Player *player, float duration);
void UpdateCamera(Camera *cam, Player *player);
void ReportTarget(Player *reporter, Player *target);
void ProcessReport(Player *target, float stunDuration);

#endif
