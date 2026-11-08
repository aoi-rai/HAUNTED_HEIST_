#include <stdio.h>
#include <SDL2/SDL.h>

#include "model2.h"
#include "room2.h"

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    // SDLの初期化
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL初期化失敗: %s\n", SDL_GetError());
        return 1;
    }

    printf("HAUNTED HEIST 起動\n");

    // 部屋の初期化
    InitRooms();

    // 部屋の情報を確認
    for (int i = 0; i < ROOM_COUNT; i++) {
        Room room = GetRoom(i);

        printf("部屋%d: X=%.1f Z=%.1f 幅=%.1f 奥行=%.1f\n",
               room.roomID,
               room.x,
               room.z,
               room.width,
               room.depth);
    }

    // SDLの終了
    SDL_Quit();

    return 0;
}
