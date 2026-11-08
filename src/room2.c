#include "src/room2.h"

// 各部屋の情報
static Room rooms[ROOM_COUNT];

// 部屋の初期化
void InitRooms(void)
{
    // 中央の大広間（4×4マスの中央2×2）
    rooms[0] = (Room){0, 0.0f, 0.0f, 20.0f, 20.0f};

    // 周囲12部屋の配置
    rooms[1]  = (Room){1, -15.0f, -15.0f, 10.0f, 10.0f};
    rooms[2]  = (Room){2,  -5.0f, -15.0f, 10.0f, 10.0f};
    rooms[3]  = (Room){3,   5.0f, -15.0f, 10.0f, 10.0f};
    rooms[4]  = (Room){4,  15.0f, -15.0f, 10.0f, 10.0f};

    rooms[5]  = (Room){5, -15.0f,  -5.0f, 10.0f, 10.0f};
    rooms[6]  = (Room){6,  15.0f,  -5.0f, 10.0f, 10.0f};

    rooms[7]  = (Room){7, -15.0f,   5.0f, 10.0f, 10.0f};
    rooms[8]  = (Room){8,  15.0f,   5.0f, 10.0f, 10.0f};

    rooms[9]  = (Room){9, -15.0f,  15.0f, 10.0f, 10.0f};
    rooms[10] = (Room){10, -5.0f,  15.0f, 10.0f, 10.0f};
    rooms[11] = (Room){11,  5.0f,  15.0f, 10.0f, 10.0f};
    rooms[12] = (Room){12, 15.0f,  15.0f, 10.0f, 10.0f};
}

// 部屋の情報を取得
Room GetRoom(int roomID)
{
    if (roomID < 0 || roomID >= ROOM_COUNT) {
        return (Room){-1, 0, 0, 0, 0};
    }

    return rooms[roomID];
}