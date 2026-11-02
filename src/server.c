#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 50000
#define BUFFER_SIZE 1024

int main(void)
{
    int server_fd;
    int client_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

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
    if(listen(server_fd, 4) < 0){
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("サーバーを起動しました。ポート番号：%d\n", PORT);
    printf("クライアントの接続を待っています...\n");

    /*クライアントからの接続を受け付ける*/
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

    if(client_fd < 0){
        perror("accept");
        close(server_fd);
        return 1;
    }

    printf("クライアントの接続完了\n");

    /*メッセージを受信*/
    int received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if(received > 0){
        buffer[received] = '\0';
        printf("メッセージ：%s\n", buffer);

        /*クライアントに返事を送信*/
        const char *message = "サーバーからの返信";

        if(send(client_fd, message, strlen(message), 0) < 0){
            perror("send");
        }
    }

    /*通信終了*/
    close(client_fd);
    close(server_fd);

    printf("サーバーを終了しました。\n");
    return 0;
}