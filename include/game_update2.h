#ifndef GAME_UPDATE2_H
#define GAME_UPDATE2_H

#include <SDL2/SDL.h>
#include "game_init2.h"

void UpdateGameFrame(
    GameInitData *game,
    const Uint8 *keys,
    float deltaTime,
    Uint32 currentTime,
    Uint32 *lastMapChange,
    Uint32 mapChangeInterval
);

// プレイヤーが発覚したとき
void ApplyDiscoveredEffect(GameInitData *game);

// NPC状態になったとき
void ApplyNpcStateEffect(GameInitData *game);

// 偽物の宝物を取ったとき
void ApplyFakeTreasureEffect(GameInitData *game);
void HandleNPCAction(GameInitData *game);

#endif
