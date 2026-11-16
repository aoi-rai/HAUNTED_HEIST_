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

// グリッド内で、配置可能なマスかどうか
static int IsValidCell(int row, int col)
{
    if (row < 0 || row >= GRID_SIZE ||
        col < 0 || col >= GRID_SIZE) {
        return 0;
    }

    if (IsCenterCell(row, col)) {
        return 0;
    }

    return 1;
}

// 小部屋の配置をランダムに決定
void RandomizeRoomPositions(Room rooms[ROOM_COUNT])
{
    // 配置済みマス
    int selected[GRID_SIZE][GRID_SIZE] = {0};

    // 大広間に隣接する8か所
    const int startCells[8][2] = {
        {1, 2}, {1, 3},
        {4, 2}, {4, 3},
        {2, 1}, {3, 1},
        {2, 4}, {3, 4}
    };

    // 最初の部屋をランダムに決定
    int start = rand() % 8;

    int row = startCells[start][0];
    int col = startCells[start][1];

    selected[row][col] = 1;

    // 選択済み部屋数
    int selectedCount = 1;

    // 12部屋になるまで配置
    while (selectedCount < SMALL_ROOM_COUNT) {

        // 次に配置できる候補
        int frontier[GRID_SIZE * GRID_SIZE][2];
        int frontierCount = 0;

        // 配置済みの部屋を調べる
        for (int r = 0; r < GRID_SIZE; r++) {
            for (int c = 0; c < GRID_SIZE; c++) {

                if (!selected[r][c]) {
                    continue;
                }

                // 上下左右の4方向
                const int directions[4][2] = {
                    {-1, 0},
                    {1, 0},
                    {0, -1},
                    {0, 1}
                };

                for (int d = 0; d < 4; d++) {

                    int nr = r + directions[d][0];
                    int nc = c + directions[d][1];

                    if (!IsValidCell(nr, nc)) {
                        continue;
                    }

                    if (selected[nr][nc]) {
                        continue;
                    }

                    // 候補の重複を確認
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

        // 候補がなくなった場合
        if (frontierCount == 0) {
            printf("ランダム配置に失敗しました\n");
            return;
        }

        // 候補から1か所をランダムに選ぶ
        int choice = rand() % frontierCount;

        row = frontier[choice][0];
        col = frontier[choice][1];

        selected[row][col] = 1;
        selectedCount++;
    }

    // 選択した座標を部屋情報に反映
    int roomID = 1;

    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {

            if (!selected[r][c]) {
                continue;
            }

            float x = (c - 2.5f) * ROOM_SIZE;
            float z = (r - 2.5f) * ROOM_SIZE;

            rooms[roomID] = (Room){
                roomID,
                x,
                z,
                ROOM_SIZE,
                ROOM_SIZE
            };

            printf(
                "ランダム配置 部屋%d: X=%.1f Z=%.1f\n",
                roomID,
                x,
                z
            );

            roomID++;
        }
    }

    printf("小部屋のランダム配置完了: %d部屋\n",
           selectedCount);
}
