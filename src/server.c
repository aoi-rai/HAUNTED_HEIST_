
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 50000
#define MAX_PLAYERS 4
#define BUFFER_SIZE 1024

int send_all(int fd, const char *data, int length)
{
    int total = 0;

    while (total < length) {
        int sent = send(
            fd,
            data + total,
            length - total,
            MSG_NOSIGNAL
        );

        if (sent <= 0) {
            return -1;
        }

        total += sent;
    }

    return 0;
}

void disconnect_player(
    int client_fds[],
    int ready[],
    char line_buffers[][BUFFER_SIZE],
    int line_lengths[],
    int index
)
{
    if (client_fds[index] != -1) {
        printf("Player %d を切断しました。\n", index + 1);
        close(client_fds[index]);
        client_fds[index] = -1;
    }

    ready[index] = 0;
    line_lengths[index] = 0;
    line_buffers[index][0] = '\0';

    char message[64];

    int length = snprintf(
        message,
        sizeof(message),
        "DISCONNECT,%d\n",
        index + 1
    );

    if (length > 0 && length < (int)sizeof(message)) {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (i != index && client_fds[i] != -1) {
                send_all(client_fds[i], message, length);
            }
        }
    }
}

/* 現在の接続人数・準備状態・ホストIDを通知 */
void broadcast_status(int client_fds[], int ready[])
{
    char message[BUFFER_SIZE];

    while (1) {
        int count = 0;
        int can_start = 1;
        int host_id = 0;

        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (client_fds[i] != -1) {
                count++;

                if (host_id == 0) {
                    host_id = i + 1;
                }

                if (!ready[i]) {
                    can_start = 0;
                }
            }
        }

        if (count < 2) {
            can_start = 0;
        }

        int length = snprintf(
            message,
            sizeof(message),
            "STATUS,%d,%d,%d,%d,%d,%d,%d\n",
            count,
            ready[0],
            ready[1],
            ready[2],
            ready[3],
            can_start,
            host_id
        );

        if (length <= 0 || length >= (int)sizeof(message)) {
            return;
        }

        int disconnected = 0;

        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (client_fds[i] != -1) {
                if (send_all(client_fds[i], message, length) < 0) {
                    printf(
                        "Player %d への状態通知に失敗しました。\n",
                        i + 1
                    );

                    close(client_fds[i]);
                    client_fds[i] = -1;
                    ready[i] = 0;
                    disconnected = 1;
                }
            }
        }

        if (!disconnected) {
            break;
        }
    }
}

void broadcast_message(
    int client_fds[],
    int ready[],
    char line_buffers[][BUFFER_SIZE],
    int line_lengths[],
    const char *message,
    int length
)
{
    int disconnected = 0;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (client_fds[i] != -1) {
            if (send_all(client_fds[i], message, length) < 0) {
                printf(
                    "Player %d への送信が失敗しました。\n",
                    i + 1
                );

                disconnect_player(
                    client_fds,
                    ready,
                    line_buffers,
                    line_lengths,
                    i
                );

                disconnected = 1;
            }
        }
    }

    if (disconnected) {
        broadcast_status(client_fds, ready);
    }
}

void process_message(
    int client_fds[],
    int ready[],
    char line_buffers[][BUFFER_SIZE],
    int line_lengths[],
    int player_index,
    int *game_started,
    const char *buffer
)
{
    char message[BUFFER_SIZE + 64];

    /* 現在のホストを取得 */
    int host_index = -1;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (client_fds[i] != -1) {
            host_index = i;
            break;
        }
    }

    printf("Player %d: %s\n", player_index + 1, buffer);

    if (strcmp(buffer, "READY") == 0) {
        ready[player_index] = 1;
        broadcast_status(client_fds, ready);
    }
    else if (strcmp(buffer, "UNREADY") == 0) {
        ready[player_index] = 0;
        broadcast_status(client_fds, ready);
    }
    else if (strcmp(buffer, "START") == 0) {
        int count = 0;
        int all_ready = 1;

        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (client_fds[i] != -1) {
                count++;

                if (!ready[i]) {
                    all_ready = 0;
                }
            }
        }

        printf(
            "[START CHECK] player=%d host=%d count=%d ready=%d started=%d\n",
            player_index + 1,
            host_index + 1,
            count,
            all_ready,
            *game_started
        );

        if (player_index == host_index &&
            count >= 2 &&
            all_ready &&
            !(*game_started)) {

            const char start_message[] = "START\n";

            *game_started = 1;

            broadcast_message(
                client_fds,
                ready,
                line_buffers,
                line_lengths,
                start_message,
                sizeof(start_message) - 1
            );

            printf(
                "ホストPlayer %dがゲームを開始しました。\n",
                player_index + 1
            );
        }
    }
    else if (strncmp(buffer, "MAP,", 4) == 0) {
        if (player_index == host_index) {
            int length = snprintf(
                message,
                sizeof(message),
                "Player %d: %s\n",
                player_index + 1,
                buffer
            );

            if (length > 0 &&
                length < (int)sizeof(message)) {

                broadcast_message(
                    client_fds,
                    ready,
                    line_buffers,
                    line_lengths,
                    message,
                    length
                );
            }
        }
        else {
            printf(
                "Player %d のMAP送信を拒否しました。\n",
                player_index + 1
            );
        }
    }
    else {
        int length = snprintf(
            message,
            sizeof(message),
            "Player %d: %s\n",
            player_index + 1,
            buffer
        );

        if (length > 0 &&
            length < (int)sizeof(message)) {

            broadcast_message(
                client_fds,
                ready,
                line_buffers,
                line_lengths,
                message,
                length
            );
        }
    }
}

int main(void)
{
    int server_fd;
    int client_fds[MAX_PLAYERS];
    int ready[MAX_PLAYERS] = {0};
    int game_started = 0;

    char line_buffers[MAX_PLAYERS][BUFFER_SIZE] = {{0}};
    int line_lengths[MAX_PLAYERS] = {0};

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    fd_set read_fds;

    char receive_buffer[BUFFER_SIZE];

    for (int i = 0; i < MAX_PLAYERS; i++) {
        client_fds[i] = -1;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;

    setsockopt(
        server_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &opt,
        sizeof(opt)
    );

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(
        server_fd,
        (struct sockaddr *)&server_addr,
        sizeof(server_addr)
    ) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, MAX_PLAYERS) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("サーバーを起動しました。\n");
    printf("最大プレイヤー数：%d\n", MAX_PLAYERS);

    while (1) {
        FD_ZERO(&read_fds);

        FD_SET(server_fd, &read_fds);

        int max_fd = server_fd;

        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (client_fds[i] != -1) {
                FD_SET(client_fds[i], &read_fds);

                if (client_fds[i] > max_fd) {
                    max_fd = client_fds[i];
                }
            }
        }

        int activity = select(
            max_fd + 1,
            &read_fds,
            NULL,
            NULL,
            NULL
        );

        if (activity < 0) {
            perror("select");
            break;
        }

        /* 新規接続 */
        if (FD_ISSET(server_fd, &read_fds)) {
            socklen_t client_len = sizeof(client_addr);

            int new_fd = accept(
                server_fd,
                (struct sockaddr *)&client_addr,
                &client_len
            );

            if (new_fd < 0) {
                perror("accept");
            }
            else {
                int slot = -1;

                for (int i = 0; i < MAX_PLAYERS; i++) {
                    if (client_fds[i] == -1) {
                        slot = i;
                        break;
                    }
                }

                if (slot == -1) {
                    const char *full_message = "Server is full\n";

                    send_all(
                        new_fd,
                        full_message,
                        strlen(full_message)
                    );

                    close(new_fd);

                    printf("接続上限のため接続を拒否しました。\n");
                }
                else {
                    client_fds[slot] = new_fd;
                    ready[slot] = 0;
                    line_lengths[slot] = 0;
                    line_buffers[slot][0] = '\0';

                    printf("Player %d が接続しました。\n", slot + 1);

                    char id_message[BUFFER_SIZE];

                    snprintf(
                        id_message,
                        sizeof(id_message),
                        "PLAYER_ID:%d\n",
                        slot + 1
                    );

                    if (send_all(
                        new_fd,
                        id_message,
                        strlen(id_message)
                    ) < 0) {
                        disconnect_player(
                            client_fds,
                            ready,
                            line_buffers,
                            line_lengths,
                            slot
                        );
                    }

                    broadcast_status(client_fds, ready);
                }
            }
        }

        /* 接続中のプレイヤーからの通信 */
        for (int i = 0; i < MAX_PLAYERS; i++) {
            int fd = client_fds[i];

            if (fd == -1 || !FD_ISSET(fd, &read_fds)) {
                continue;
            }

            int received = recv(
                fd,
                receive_buffer,
                sizeof(receive_buffer),
                0
            );

            if (received <= 0) {
                disconnect_player(
                    client_fds,
                    ready,
                    line_buffers,
                    line_lengths,
                    i
                );

                game_started = 0;

                broadcast_status(client_fds, ready);
                continue;
            }

            for (int j = 0; j < received; j++) {
                char c = receive_buffer[j];

                if (c == '\n') {
                    line_buffers[i][line_lengths[i]] = '\0';

                    if (line_lengths[i] > 0 &&
                        line_buffers[i][line_lengths[i] - 1] == '\r') {
                        line_buffers[i][line_lengths[i] - 1] = '\0';
                    }

                    process_message(
                        client_fds,
                        ready,
                        line_buffers,
                        line_lengths,
                        i,
                        &game_started,
                        line_buffers[i]
                    );

                    line_lengths[i] = 0;
                    line_buffers[i][0] = '\0';

                    if (client_fds[i] == -1) {
                        break;
                    }
                }
                else {
                    if (line_lengths[i] >= BUFFER_SIZE - 1) {
                        printf(
                            "Player %d のメッセージが長すぎます。\n",
                            i + 1
                        );

                        disconnect_player(
                            client_fds,
                            ready,
                            line_buffers,
                            line_lengths,
                            i
                        );

                        game_started = 0;
                        broadcast_status(client_fds, ready);
                        break;
                    }

                    line_buffers[i][line_lengths[i]] = c;
                    line_lengths[i]++;
                }
            }
        }
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (client_fds[i] != -1) {
            close(client_fds[i]);
        }
    }

    close(server_fd);

    printf("サーバーを終了しました。\n");

    return 0;
}
