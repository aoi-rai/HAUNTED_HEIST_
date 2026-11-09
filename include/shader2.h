#ifndef SHADER_H
#define SHADER_H

// シェーダーの読み込み・作成
unsigned int LoadShaders(
    const char *vertexPath,
    const char *fragmentPath
);

// シェーダーの解放
void FreeShaders(unsigned int programID);

#endif
