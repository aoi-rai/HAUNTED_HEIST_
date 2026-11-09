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

/*データを最後まで送信する関数*/
int send_all(int fd, const char *data, int length)
{
    int total = 0;
    while(total < length){
        int sent = send(fd, data + total, length - total, 0);

        if(sent <= 0){
            return -1;
        }
        total += sent;
    }
    return 0;
}

/*全プレイヤーにメッセージを配信する関数*/
void broadcast_message(int client_fds[], const char *message, int length)
{
    for(int i = 0;i < MAX_PLAYERS;i++){
        if(client_fds[i] != -1){
            if(send_all(client_fds[i], message,length) < 0){
                printf("Player %d への送信が失敗しました。\n", i + 1);
                close(client_fds[i]);
                client_fds[i] = -1;
                }
        }
    }
}

int main(void)
{
    int server_fd;
    int client_fds[MAX_PLAYERS];
    
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    fd_set read_fds;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE + 64];

    /*クライアント管理用の配列の初期化*/
    for(int i = 0;i < MAX_PLAYERS; i++){
        client_fds[i] = -1;
    }

    /*ソケットの作成*/
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(server_fd < 0){
        perror("socket");
        return 1;
    }

    /*サーバーのアドレス設定*/
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    /*ポート番号割当*/
    if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("bind");
        close(server_fd);
        return 1;
    }

    /*接続待機*/
    if(listen(server_fd, MAX_PLAYERS) < 0){
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("サーバーを起動しました。\n");
    printf("最大プレイヤー数：%d\n", MAX_PLAYERS);

    /*通信管理ループ*/
    while(1){
        FD_ZERO(&read_fds);

        /*サーバーソケットを監視対象に追加*/
        FD_SET(server_fd, &read_fds);

        int max_fd = server_fd;

        /*接続中のクライアントを監視対象に追加*/
        for(int i = 0;i < MAX_PLAYERS; i++){
            if(client_fds[i] != -1){
                FD_SET(client_fds[i], &read_fds);
                if(client_fds[i] > max_fd){
                    max_fd = client_fds[i];
                }
            }
        }

    /*通信開始まで待機*/
    int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
    if(activity < 0){
        perror("select");
        break;
    }

    /*新しい接続が来たか確認*/
    if(FD_ISSET(server_fd, &read_fds)){
        socklen_t client_len = sizeof(client_addr);
        int new_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if(new_fd < 0){
            perror("accept");
            continue;
        }

        /*クライアント数に空きがあるか探す*/
        int slot = -1;

        for(int i = 0;i < MAX_PLAYERS; i++){
            if(client_fds[i] == -1){
                slot = i;
                break;
            }
        }
        if(slot == -1){
            const char *full_message = "Server is full";
            send_all(new_fd, full_message, strlen(full_message));
            close(new_fd);
            printf("プレイヤー数が上限に達したため、接続を拒否しました。\n");
        }
        else{
            client_fds[slot] = new_fd;

            printf("Player %d が接続しました。\n", slot + 1);

            /*プレイヤーIDを送信*/
            char id_message[BUFFER_SIZE];
            snprintf(id_message, sizeof(id_message), "PLAYER_ID:%d\n", slot + 1);
            send_all(new_fd, id_message, strlen(id_message));
        }
    }

    /*接続中のプレイヤーからの通信の確認*/
    for(int i = 0;i < MAX_PLAYERS; i++){
        int fd = client_fds[i];
        if(fd != -1 && FD_ISSET(fd, &read_fds)){
            int received = recv(fd, buffer, BUFFER_SIZE - 1, 0);
            if(received <= 0){
                printf("Player %d が切断しました。\n", i + 1);
                close(fd);
                client_fds[i] = -1;
            }
            else{
                buffer[received] = '\0';
                /*末尾の改行を削除*/
                buffer[strcspn(buffer, "\r\n")] = '\0';
                printf("Player %d: %s\n", i + 1, buffer);

                /*配信メッセージを作成*/
                int length = snprintf(message, sizeof(message), "Player %d: %s\n", i + 1, buffer);

                /*全員に配信*/
                if(length > 0 && length < sizeof(message)){
                    broadcast_message(client_fds, message, length);
                }
            }
        }
    }
}

    /*通信終了*/
    for(int i = 0;i < MAX_PLAYERS; i++){
        if(client_fds[i] != -1){
            close(client_fds[i]);
        }
    }
    close(server_fd);

    printf("サーバーを終了しました。\n");
    return 0;
}