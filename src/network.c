
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#include "network.h"

#define MAX_PLAYERS 4
#define RECEIVE_BUFFER_SIZE 4096

static int server_fd = -1;
static int client_fd = -1;
static int player_fds[MAX_PLAYERS] = {-1, -1, -1, -1};
static int player_id = -1;

/* 受信データを一時保存するバッファ */
static char receive_buffer[RECEIVE_BUFFER_SIZE];
static int receive_length = 0;

/* サーバーの起動 */
int Network_InitServer(int port)
{
    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        perror("socket");
        return -1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        server_fd = -1;
        return -1;
    }

    if (listen(server_fd, MAX_PLAYERS - 1) < 0) {
        perror("listen");
        close(server_fd);
        server_fd = -1;
        return -1;
    }

    printf("サーバーを起動しました。\n");

    return 0;
}

/* サーバーへの接続 */
int Network_Connect(const char *ip, int port)
{
    struct sockaddr_in server_addr;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (client_fd < 0) {
        perror("socket");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        printf("IPアドレスが不正です。\n");
        close(client_fd);
        client_fd = -1;
        return -1;
    }

    if (connect(client_fd, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0) {
        perror("connect");
        close(client_fd);
        client_fd = -1;
        return -1;
    }

    /* サーバーからプレイヤーIDを受信 */
    char id_buffer[64];
    int pos = 0;
    char ch;

    while (pos < (int)sizeof(id_buffer) - 1) {
        int received = recv(client_fd, &ch, 1, 0);

        if (received <= 0) {
            perror("プレイヤーID受信");
            close(client_fd);
            client_fd = -1;
            return -1;
        }

        if (ch == '\n') {
            break;
        }

        id_buffer[pos++] = ch;
    }

    id_buffer[pos] = '\0';

    if (sscanf(id_buffer, "PLAYER_ID:%d", &player_id) != 1) {
        printf("プレイヤーIDの形式が不正です: %s\n", id_buffer);
        close(client_fd);
        client_fd = -1;
        player_id = -1;
        return -1;
    }

    /* 受信バッファを初期化 */
    receive_length = 0;

    printf("サーバーに接続しました。プレイヤーID:%d\n", player_id);

    return 0;
}

/* データ送信 */
int Network_Send(const char *data)
{
    if (client_fd == -1 || data == NULL) {
        return -1;
    }

    int length = strlen(data);
    int total = 0;

    while (total < length) {
        int sent = send(
            client_fd,
            data + total,
            length - total,
            MSG_NOSIGNAL
        );

        if (sent <= 0) {
            perror("send");
            return -1;
        }

        total += sent;
    }

    return 0;
}

/*
 * データ受信
 *
 * 戻り値：
 *  正の値：受信した1行の文字数
 *  0      ：まだ1行分のデータが届いていない
 *  -1     ：通信エラーまたは切断
 *
 * 改行文字は返すデータに含まれない。
 */
int Network_Receive(char *buffer, int buffer_size)
{
    if (client_fd == -1 || buffer == NULL || buffer_size <= 1) {
        return -1;
    }

    /* すでに改行まで受信できているか確認 */
    char *newline = memchr(receive_buffer, '\n', receive_length);

    if (newline == NULL) {
        /* 新しいデータが届いているか確認 */
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(client_fd, &read_fds);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        int result = select(
            client_fd + 1,
            &read_fds,
            NULL,
            NULL,
            &timeout
        );

        if (result < 0) {
            perror("select");
            return -1;
        }

        if (result == 0) {
            return 0;
        }

        /* 受信バッファの空き容量を確認 */
        if (receive_length >= RECEIVE_BUFFER_SIZE - 1) {
            printf("受信バッファがいっぱいです。\n");
            close(client_fd);
            client_fd = -1;
            receive_length = 0;
            return -1;
        }

        int received = recv(
            client_fd,
            receive_buffer + receive_length,
            RECEIVE_BUFFER_SIZE - 1 - receive_length,
            0
        );

        if (received <= 0) {
            printf("サーバーとの接続が切れました。\n");
            close(client_fd);
            client_fd = -1;
            receive_length = 0;
            return -1;
        }

        receive_length += received;

        /* 改行まで届いていなければ次回に続ける */
        newline = memchr(receive_buffer, '\n', receive_length);

        if (newline == NULL) {
            return 0;
        }
    }

    /* 最初の改行までを1行として取り出す */
    int line_length = (int)(newline - receive_buffer);

    /* CRLFにも対応 */
    if (line_length > 0 && receive_buffer[line_length - 1] == '\r') {
        line_length--;
    }

    /* 呼び出し側のバッファに収まるか確認 */
    if (line_length >= buffer_size) {
        printf("受信データがバッファサイズを超えました。\n");
        return -1;
    }

    memcpy(buffer, receive_buffer, line_length);
    buffer[line_length] = '\0';

    /* 処理済みデータを取り除き、残りを保存 */
    int consumed = (int)(newline - receive_buffer) + 1;
    int remaining = receive_length - consumed;

    memmove(
        receive_buffer,
        receive_buffer + consumed,
        remaining
    );

    receive_length = remaining;

    return line_length;
}

/* 通信終了 */
void Network_Close(void)
{
    if (client_fd != -1) {
        close(client_fd);
        client_fd = -1;
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (player_fds[i] != -1) {
            close(player_fds[i]);
            player_fds[i] = -1;
        }
    }

    if (server_fd != -1) {
        close(server_fd);
        server_fd = -1;
    }

    player_id = -1;
    receive_length = 0;

    printf("ネットワークを終了しました。\n");
}

/* プレイヤーID取得 */
int Network_GetPlayerID(void)
{
    return player_id;
}
