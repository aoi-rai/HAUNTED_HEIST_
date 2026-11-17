
#include <stdio.h>
#include <stdlib.h>

#include "map_random2.h"

#define GRID_SIZE 6
#define ROOM_SIZE 10.0f
#define SMALL_ROOM_COUNT 12

// 大広間と重なる中央4マスかどうか
static int IsCenterCell(int row, int col)
{
    return (row == 2 || row == 3) &&
           (col == 2 || col == 3);
}

// 配置可能なマスかどうか
static int IsValidCell(int row, int col)
{
    if (row < 0 || row >= GRID_SIZE ||
        col < 0 || col >= GRID_SIZE) {
        return 0;
    }

    return !IsCenterCell(row, col);
}

// 小部屋の配置をランダムに決定
void RandomizeRoomPositions(Room rooms[ROOM_COUNT])
{
    int selected[GRID_SIZE][GRID_SIZE] = {0};

    // 大広間に隣接する8か所
    const int startCells[8][2] = {
        {1, 2}, {1, 3},
        {4, 2}, {4, 3},
        {2, 1}, {3, 1},
        {2, 4}, {3, 4}
    };

    // 最初の部屋を決定
    int start = rand() % 8;

    int row = startCells[start][0];
    int col = startCells[start][1];

    selected[row][col] = 1;

    int selectedCount = 1;

    // 12部屋になるまで配置
    while (selectedCount < SMALL_ROOM_COUNT) {

        int frontier[GRID_SIZE * GRID_SIZE][2];
        int frontierCount = 0;

        const int directions[4][2] = {
            {-1, 0},
            {1, 0},
            {0, -1},
            {0, 1}
        };

        for (int r = 0; r < GRID_SIZE; r++) {
            for (int c = 0; c < GRID_SIZE; c++) {

                if (!selected[r][c]) {
                    continue;
                }

                for (int d = 0; d < 4; d++) {

                    int nr = r + directions[d][0];
                    int nc = c + directions[d][1];

                    if (!IsValidCell(nr, nc)) {
                        continue;
                    }

                    if (selected[nr][nc]) {
                        continue;
                    }

                    int duplicate = 0;

                    for (int k = 0; k < frontierCount; k++) {
                        if (frontier[k][0] == nr &&
                            frontier[k][1] == nc) {
                            duplicate = 1;
                            break;
                        }
                    }

                    if (!duplicate) {
                        frontier[frontierCount][0] = nr;
                        frontier[frontierCount][1] = nc;
                        frontierCount++;
                    }
                }
            }
        }

        if (frontierCount == 0) {
            printf("ランダム配置に失敗しました\n");
            return;
        }

        int choice = rand() % frontierCount;

        row = frontier[choice][0];
        col = frontier[choice][1];

        selected[row][col] = 1;
        selectedCount++;
    }

    // 選択した12マスを配列に保存
    int cells[SMALL_ROOM_COUNT][2];
    int cellCount = 0;

    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {

            if (!selected[r][c]) {
                continue;
            }

            cells[cellCount][0] = r;
            cells[cellCount][1] = c;
            cellCount++;
        }
    }

    // 部屋IDと配置先の対応をランダムにする
    for (int i = SMALL_ROOM_COUNT - 1; i > 0; i--) {

        int j = rand() % (i + 1);

        int tempRow = cells[i][0];
        int tempCol = cells[i][1];

        cells[i][0] = cells[j][0];
        cells[i][1] = cells[j][1];

        cells[j][0] = tempRow;
        cells[j][1] = tempCol;
    }

    // 部屋IDを維持して座標だけ変更
    for (int roomID = 1; roomID < ROOM_COUNT; roomID++) {

        int index = roomID - 1;

        int r = cells[index][0];
        int c = cells[index][1];

        rooms[roomID].roomID = roomID;

        rooms[roomID].x =
            (c - 2.5f) * ROOM_SIZE;

        rooms[roomID].z =
            (r - 2.5f) * ROOM_SIZE;

        rooms[roomID].width = ROOM_SIZE;
        rooms[roomID].depth = ROOM_SIZE;

        printf(
            "ランダム配置 部屋%d: X=%.1f Z=%.1f\n",
            roomID,
            rooms[roomID].x,
            rooms[roomID].z
        );
    }

    printf(
        "小部屋のランダム配置完了: %d部屋\n",
        selectedCount
    );
}
