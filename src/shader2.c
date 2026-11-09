#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>

#include "shader2.h"

// ファイルの読み込み
char *ReadShaderFile(const char *filename)
{
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("シェーダー読み込み失敗: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *source = malloc(size + 1);

    if (source == NULL) {
        fclose(file);
        return NULL;
    }

    fread(source, 1, size, file);
    source[size] = '\0';

    fclose(file);

    return source;
}

// シェーダーのコンパイル
GLuint CompileShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        char log[512];

        glGetShaderInfoLog(shader, 512, NULL, log);
        printf("シェーダーコンパイル失敗:\n%s\n", log);

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

// シェーダープログラムの作成
unsigned int LoadShaders(
    const char *vertexPath,
    const char *fragmentPath
)
{
    char *vertexSource = ReadShaderFile(vertexPath);
    char *fragmentSource = ReadShaderFile(fragmentPath);

    if (vertexSource == NULL || fragmentSource == NULL) {
        free(vertexSource);
        free(fragmentSource);
        return 0;
    }

    GLuint vertexShader =
        CompileShader(GL_VERTEX_SHADER, vertexSource);

    GLuint fragmentShader =
        CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

    free(vertexSource);
    free(fragmentSource);

    if (vertexShader == 0 || fragmentShader == 0) {
        if (vertexShader != 0) glDeleteShader(vertexShader);
        if (fragmentShader != 0) glDeleteShader(fragmentShader);
        return 0;
    }

    GLuint program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glBindAttribLocation(program, 0, "position");
    glBindAttribLocation(program, 1, "normal");

    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        char log[512];

        glGetProgramInfoLog(program, 512, NULL, log);
        printf("シェーダーリンク失敗:\n%s\n", log);

        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

// シェーダーの解放
void FreeShaders(unsigned int programID)
{
    if (programID != 0) {
        glDeleteProgram(programID);
    }
}
