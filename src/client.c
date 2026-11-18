#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <errno.h>

#define PORT 14860
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in server_addr;
    fd_set read_fds;
    char buffer[BUFFER_SIZE];
    char send_buffer[BUFFER_SIZE];

    int player_id = -1;

    /*接続先IPアドレスを確認*/
    if(argc != 2){
        printf("Usage: %s <server IP>\n", argv[0]);
        return 1;
    }

    /*ソケット作成*/
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock < 0){
        perror("socket");
        return 1;
    }

    /*サーバーのアドレス設定*/
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    /*接続先のIPアドレス設定*/
    if(inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0){
        printf("IPアドレスが不正です。\n");
        close(sock);
        return 1;
    }

    /*サーバーに接続*/
    printf("サーバーに接続しています...\n");
    if(connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("connect");
        close(sock);
        return 1;
        }

        printf("サーバーに接続しました。\n");

        /*サーバーからプレイヤーIDを受信*/
        int pos = 0;
        while(pos < BUFFER_SIZE - 1){
        int received = recv(sock, &buffer[pos], 1, 0);
        if (received <= 0){
            printf("サーバーとの接続が切れました。\n");
            close(sock);
            return 1;
        }
        if(buffer[pos] == '\n'){
            pos++;
            break;
        }
        pos++;
        }

        buffer[pos] = '\0';
        /*サーバーが満員の場合*/
        if(strncmp(buffer, "SERVER_FULL", 11) == 0){
            printf("サーバーは満員です。\n");
            close(sock);
            return 1;
        }
        /*Player IDを取得*/
        if(sscanf(buffer, "PLAYER_ID:%d", &player_id) != 1){
            printf("プレイヤーIDを取得できませんでした。\n");
            close(sock);
            return 1;
        }
        
        printf("\n");
        printf("=============================\n");
        printf("接続成功！\n");
        printf("あなたは Player %d です。\n", player_id);
        printf("=============================\n");

        printf("メッセージを入力してEnterで送信\n");
        printf("/quitで切断します。\n\n");

        /*通信ループ*/
        /*キーボード入力とサーバーからの受信を監視*/
        while(1){
            FD_ZERO(&read_fds);

            /*キーボードの監視*/
            FD_SET(STDIN_FILENO, &read_fds);

            /*サーバーからの通信の監視*/
            FD_SET(sock, &read_fds);

            int max_fd = sock > STDIN_FILENO ? sock : STDIN_FILENO;

            /*どちらかに入力が入るまで待つ*/
            int activity = select(max_fd + 1,&read_fds, NULL, NULL, NULL);
            if(activity < 0){
                if(errno == EINTR){
                    continue;
                }
                perror("select");
                break;
            }

            /*サーバーからメッセージを受信*/
            if(FD_ISSET(sock, &read_fds)){
                int received = recv(sock, buffer, sizeof(buffer) - 1, 0);
                if(received <= 0){
                    printf("\nサーバーとの接続が切れました。\n");
                    break;
                }
                buffer[received] = '\0';
                printf("%s", buffer);
                fflush(stdout);
            }

            /*キーボードからメッセージを入力*/
            if(FD_ISSET(STDIN_FILENO, &read_fds)){
                if(fgets(send_buffer, sizeof(send_buffer), stdin) == NULL){
                    break;
                }
                /*終了コマンド*/
                if(strcmp(send_buffer, "/quit\n") == 0 || strcmp(send_buffer, "/quit") == 0){
                    printf("切断します。\n");
                    break;
                }
                /*空行は送信しない*/
                if(strcmp(send_buffer, "\n") == 0){
                    continue;
                }
                /*メッセージをサーバーに送信*/
                int length = strlen(send_buffer);
                int total = 0;

                while(total < length){
                    int sent = send(sock, send_buffer + total, length - total, MSG_NOSIGNAL);
                    if(sent <= 0){
                        perror("send");
                        goto cleanup;
                    }
                    total += sent;
                }
            }
        }

        /*通信終了*/
        cleanup:
        close(sock);

        printf("接続を終了しました。\n");

        return 0;
}