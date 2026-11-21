#ifndef ROOM_H
#define ROOM_H

#define ROOM_COUNT 13

typedef struct {
    int roomID;
    float x;
    float z;
    float width;
    float depth;
} Room;

// 部屋の初期化
void InitRooms(void);

// 小部屋の配置を再ランダム化
void RandomizeRooms(void);

// ランダム配置の実行
void RandomizeRoomPositions(Room rooms[ROOM_COUNT]);

// 部屋の情報を取得
Room GetRoom(int roomID);

// 指定した座標に小部屋を配置
void SetRoomPosition(int roomID, float x, float z);

#endif
