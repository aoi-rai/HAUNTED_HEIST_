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

// 部屋の情報を取得
Room GetRoom(int roomID);

#endif
