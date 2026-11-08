#include <stdio.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

int LoadModel(const char *filename)
{
    const struct aiScene *scene = aiImportFile(
        filename,
        aiProcess_Triangulate |
        aiProcess_GenNormals
    );

    if (scene == NULL) {
        printf("読み込み失敗: %s\n", aiGetErrorString());
        return 0;
    }

    printf("読み込み成功: %s\n", filename);
    printf("メッシュ数: %u\n", scene->mNumMeshes);

    aiReleaseImport(scene);
    return 1;
}