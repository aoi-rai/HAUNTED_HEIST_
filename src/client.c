#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 50000
#define BUFFER_SIZE 1024

int main(void)
{
    int sock;
    struct sockaddr_in server_addr;
    fd_set read_fds;
    char buffer[BUFFER_SIZE];

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
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    /*サーバーに接続*/
    if(connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("connect");
        close(sock);
        return 1;
        }

        printf("サーバーに接続しました。\n");

        /*サーバーからプレイヤーIDを受信*/
        int received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (received <= 0){
            printf("プレイヤーIDを受信できませんでした。\n");
            close(sock);
            return 1;
        }

        buffer[received] = '\0';
        printf("あなたのID:%s\n", buffer);
        printf("終了:quit\n");

        /*通信ループ*/
        while(1){
            FD_ZERO(&read_fds);

            /*キーボードの監視*/
            FD_SET(STDIN_FILENO, &read_fds);

            /*サーバーからの通信の監視*/
            FD_SET(sock, &read_fds);

            int max_fd = sock;
            if(STDIN_FILENO > max_fd){
                max_fd = STDIN_FILENO;
            }

            /*どちらかに入力が入るまで待つ*/
            int activity = select(max_fd + 1,&read_fds, NULL, NULL, NULL);
            if(activity < 0){
                perror("select");
                break;
            }

            /*キーボード入力*/
            if(FD_ISSET(STDIN_FILENO, &read_fds)){
                if(fgets(buffer, sizeof(buffer), stdin) == NULL){
                    break;
                }
                if(strcmp(buffer, "quit\n") == 0){
                    break;
                }
                /*サーバーに送信*/
                if(send(sock, buffer, strlen(buffer), 0) <= 0){
                    perror("send");
                    break;
                }
            }

            /*サーバーからの受信*/
            if(FD_ISSET(sock, &read_fds)){
                received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
                if(received <= 0){
                    printf("サーバーとの接続が切れました。\n");
                    break;
                }
                buffer[received] = '\0';
                printf("%s", buffer);
                fflush(stdout);
            }
        }

        /*通信終了*/
        close(sock);

        printf("接続を終了しました。\n");

        return 0;
}