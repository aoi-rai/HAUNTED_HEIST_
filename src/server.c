#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <errno.h>

#define PORT 14860
#define MAX_PLAYERS 4
#define BUFFER_SIZE 1024

/*プレイヤーごとの受信途中データ*/
static char receive_buffers[MAX_PLAYERS][BUFFER_SIZE];
static int receive_lengths[MAX_PLAYERS];

/*データを最後まで送信する関数*/
int send_all(int fd, const char *data, int length)
{
    int total = 0;
    while(total < length){
        int sent = send(fd, data + total, length - total, MSG_NOSIGNAL);

        if(sent <= 0){
            return -1;
        }
        total += sent;
    }
    return 0;
}

/*プレイヤーを切断して枠を解放する関数*/
void disconnect_player(int client_fds[], int index)
{
    if(client_fds[index] != -1){
            close(client_fds[index]);
            client_fds[index] = -1;
            receive_lengths[index] = 0;

            printf("Player %d が切断しました。\n", index + 1);
    }
}

/*全プレイヤーにメッセージを配信する関数*/
void broadcast_message(int client_fds[], const char *message, int length)
{
    for(int i = 0;i < MAX_PLAYERS;i++){
        if(client_fds[i] != -1){
            if(send_all(client_fds[i], message,length) < 0){
                printf("Player %d への送信が失敗しました。\n", i + 1);
                disconnect_player(client_fds, i);
                }
        }
    }
}

/*受信した１行分のメッセージを処理*/
void process_message(int client_fds[], int player_index, const char *data){
    char message[BUFFER_SIZE + 64];
    printf("Player %d: %s\n", player_index + 1, data);
    int length = snprintf(message, sizeof(message), "Player %d: %s\n", player_index + 1, data);
    if(length > 0 && length < (int)sizeof(message)){
        broadcast_message(client_fds, message, length);
    }
}

int main(void)
{
    int server_fd;
    int client_fds[MAX_PLAYERS];
    
    struct sockaddr_in server_addr;
    fd_set read_fds;

    /*プレイヤー情報の初期化*/
    for(int i = 0;i < MAX_PLAYERS; i++){
        client_fds[i] = -1;
        receive_lengths[i] = 0;
    }

    /*ソケットの作成*/
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(server_fd < 0){
        perror("socket");
        return 1;
    }

    /*selectで監視できる範囲か確認*/
    if(server_fd >= FD_SETSIZE){
        printf("サーバーソケットが監視可能範囲を超えています。\n");
        close(server_fd);
        return 1;
    }

    /*アドレス再利用を許可*/
    int opt = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        perror("setsockopt");
        close(server_fd);
        return -1;
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    /*IPアドレスとポート番号割当*/
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
    printf("ポート番号:%d\n", PORT);
    printf("最大プレイヤー数：%d\n", MAX_PLAYERS);
    printf("プレイヤーの接続を待っています...\n");

    /*通信管理ループ*/
    while(1){
        FD_ZERO(&read_fds);

        /*サーバーソケットを監視対象に追加*/
        FD_SET(server_fd, &read_fds);

        int max_fd = server_fd;

        /*接続中のプレイヤーを監視対象に追加*/
        for(int i = 0;i < MAX_PLAYERS; i++){
            if(client_fds[i] != -1){
                FD_SET(client_fds[i], &read_fds);
                if(client_fds[i] > max_fd){
                    max_fd = client_fds[i];
                }
            }
        }

    /*データ受信または新規接続が来るまで待機*/
    int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
    if(activity < 0){
        if(errno == EINTR){
            continue;
        }
        perror("select");
        break;
    }

    /*新しいプレイヤーが来たか接続確認*/
    if(FD_ISSET(server_fd, &read_fds)){
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int new_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if(new_fd < 0){
            perror("accept");
            continue;
        }

        /*selectで監視できる範囲か確認*/
        if(new_fd >= FD_SETSIZE){
            printf("接続を受け付けられません。\n");
            close(new_fd);
            continue;
        }

        int slot = -1;

        /*プレイヤー数に空きがあるか探す*/
        for(int i = 0;i < MAX_PLAYERS; i++){
            if(client_fds[i] == -1){
                slot = i;
                break;
            }
        }
        /*4人接続済みの場合*/
        if(slot == -1){
            const char *full_message = "SERVER_FULL\n";
            send_all(new_fd, full_message, strlen(full_message));
            close(new_fd);
            printf("プレイヤー数が上限に達したため、接続を拒否しました。\n");
        }
        else{
            client_fds[slot] = new_fd;
            receive_lengths[slot] = 0;

            printf("Player %d が接続しました。\n", slot + 1);

            /*プレイヤーIDを送信*/
            char id_message[64];
            snprintf(id_message, sizeof(id_message), "PLAYER_ID:%d\n", slot + 1);
            if(send_all(new_fd, id_message, strlen(id_message)) < 0){
                printf("Player %d へのID送信に失敗しました。\n", slot + 1);
                disconnect_player(client_fds, slot);
            }
        }
    }

    /*接続中のプレイヤーからデータを送信*/
    for(int i = 0;i < MAX_PLAYERS; i++){
        int fd = client_fds[i];
        if(fd == -1 || !FD_ISSET(fd, &read_fds)){
            continue;
        }
        char buffer[BUFFER_SIZE];
            int received = recv(fd, buffer, sizeof(buffer), 0);
            /*切断または受信エラー*/
            if(received <= 0){
                disconnect_player(client_fds, i);
                continue;
            }
            /*受信データを１文字ずつ処理*/
            int disconnected = 0;
            for(int j = 0;j < received;j++){
                char ch = buffer[j];
                if(ch == '\r'){
                    continue;
                }
                /*改行が来たら１つのメッセージとして処理*/
                if(ch == '\n'){
                    receive_buffers[i][receive_lengths[i]] = '\0';
                    if(receive_lengths[i] > 0){
                        process_message(client_fds, i, receive_buffers[i]);
                    }
                    receive_lengths[i] = 0;
                }else{
                    /*バッファの上限を確認*/
                    if(receive_lengths[i] >= BUFFER_SIZE - 1){
                        printf("Player %d の受信データが長すぎます。\n", i + 1);
                        disconnect_player(client_fds, i);
                        disconnected = 1;
                        break;
                    }
                    receive_buffers[i][receive_lengths[i]++] = ch;
                }
            }
            if(disconnected){
                continue;
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