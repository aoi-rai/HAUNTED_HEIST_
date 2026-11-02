#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 50000
#define BUFFER_SIZE 1024

int main(void)
{
    int sock;
    struct sockaddr_in server_addr;
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

        /*メッセージを送信*/
        const char *message = "Hello Server!";
        send(sock, message, strlen(message), 0);
        printf("サーバーにメッセージを送信しました。\n");

        /*サーバーから返信を受信*/
        int received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (received > 0){
            buffer[received] = '\0';
            printf("サーバーからの返信：%s\n", buffer);
        }

        /*通信終了*/
        close(sock);

        return 0;
}