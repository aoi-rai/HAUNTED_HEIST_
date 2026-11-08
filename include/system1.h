/*
 *  ファイル名	: system.h
 *  機能	: 共通変数，外部関数の定義
 */

#include <SDL2/SDL.h>

/* マップサイズ(ピクセル) */
#define MAP_WIDTH 720
#define MAP_HEIGHT 720

/* ウインドウサイズ(ピクセル) */
#define WD_WIDTH 720
#define WD_HEIGHT 720

/* プレイヤー構造体 */
typedef struct {
    int playerdId;    // プレイヤーを識別する番号
    float positionX;  // X座標
    float positionY;  // Y座標
    float directionX; // 向いている方向X
    float directionY; // 向いている方向Y
    int isActive;     // 行動可能かどうか
    float stunTime;   // 行動不能の残り時間
    int hasTreasure;  // 本物のお宝を所持しているか
    int haskey;       // 鍵を所持しているか
} Player;

/* NPC構造体 */
typedef struct {
    int npcId;        // NPCを識別する番号
    float positionX;  // X座標
    float positionY;  // Y座標
    int moveState;    // 移動状態
    int treasureID;   // 所持している宝箱の番号
} NPC;

/* 宝箱構造体 */
typedef struct {
    int chestId;      // 宝箱を識別する番号
    float positionX;  // X座標
    float positionY;  // Y座標
    int chestType;    // 宝箱の種類
    int isOpened;     // 開封済みかどうか
    int ownerId;      // 現在の所持者
} TreasureChest;

/* 部屋構造体 */
typedef struct {
    int roomId;       // 部屋を識別する番号
    int roomType;     // 部屋の種類
    int interiorId;   // 内装の種類
    int furnitureId;  // 家具の種類
} Room;

/* カメラ構造体 */
typedef struct {
    float x, y;      // カメラの左上座標
    float distance;  // カメラの距離
    float height;    // 高さ
    float angle;     // 角度
} Camera;

/* end of system.h */
