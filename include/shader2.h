#ifndef SHADER_H
#define SHADER_H

// シェーダーの読み込み・作成
unsigned int LoadShaders(
    const char *vertexPath,
    const char *fragmentPath
);

// シェーダーの解放
void FreeShaders(unsigned int programID);

// シェーダーにカメラ行列を設定
void SetShaderMatrices(
    unsigned int shaderProgram,
    const float view[16],
    const float projection[16]
);


#endif
