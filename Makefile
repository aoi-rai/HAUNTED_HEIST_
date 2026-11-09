
CC = gcc

# コンパイル設定
CFLAGS = -Wall -Wextra -Iinclude `sdl2-config --cflags`

# リンク設定
LIBS = `sdl2-config --libs` -lassimp -lGL -lGLEW

# 実行ファイル名
TARGET = haunted_heist

# ソースファイル
SRCS = src/main.c \
       src/model2.c \
       src/map2.c \
       src/map_random2.c \
       src/room2.c \
       src/collision2.c \
       src/shader2.c \
       src/netwark.c

# オブジェクトファイル
OBJS = $(SRCS:.c=.o)

# ビルド
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

# コンパイル
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# クリーン
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
