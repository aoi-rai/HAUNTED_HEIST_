#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <SDL2/SDL.h>

// コントローラーの初期化と接続
int InitController(void);

// コントローラーの解放
void CloseController(void);

// スティックやボタンの入力取得
void PollControllerInput(float *inputX, float *inputY, float *rightStickX, int *dashPressed, int *jumpPressed);

#endif