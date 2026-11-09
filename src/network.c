#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#include "network.h"

#define MAX_PLAYERS 4

static int server_fd = -1;
static int client_fd = -1;
static int player_fds[MAX_PLAYERS] = {-1,-1,-1,-1};;
static int player_id = -1;

/*サーバーの起動*/
int Network_InitServer(int port)
{
    struct sockaddr_in server_addr;
    server_fd = soclet(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0){
        perror("socket");
        return -1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("bind");
        close(server_fd);
        server_fd = -1;
        return -1;
    }

    if(listen(server_fd, MAX_PLAYERS - 1) < 0){
        perror("listen");
        close(server_fd);
        server_fd = -1;
        return -1;
    }
    printf("サーバーを起動しました。\n");
    return 0;
}

/*サーバーへの接続*/
int Network_Connect(const char *ip, int port)
{
    struct sockaddr_in server_addr;
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd < 0){
        perror("socket");
        return -1;
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if(inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0){
        printf("IPアドレスが不正です。\n");
        close(client_fd);
        client_fd = -1;
        return -1;
    }

    if(connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("connect");
        close(client_fd);
        client_fd = -1;
        return -1;
    }

    printf("サーバーに接続しました。\n");
    return 0;
}

/*データ送信*/
int Network_Send(const char *data)
{
    if(client_fd == -1 || data == NULL){
        return -1;
    }
    int length = strlen(data);
    int total = 0;

    while(total < length){
        int sent = send(client_fd, data + total, length - total, MSG_NOSIGNAL);
        if(sent <= 0){
            perror("send");
            return -1;
        }
        total += sent;
    }
    return 0;
}

/*データ受信*/
int Network_Receive(char *buffer, int buffer_size)
{
    if(client_fd == -1 || buffer == NULL || buffer_size <= 1){
        return -1;
    }
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(client_fd, &read_fds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    int result = select(client_fd + 1, &read_fds, NULL, NULL, &timeout);
    if(result < 0){
        perror("select");
        return -1;
    }

    if(result == 0){
        return 0;
    }
    int received = recv(client_fd, buffer, buffer_size - 1, 0);

    if(received <= 0){
        printf("サーバーとの接続が切れました。\n");
        close(client_fd);
        client_fd = -1;
        return -1;
    }
    buffer[received] = '\0';
    return received;
}

/*通信終了*/
void Network_close(void)
{
    if(client_fd != -1){
        close(client_fd);
        client_fd = -1;
    }
    for(int i = 0;i < MAX_PLAYERS;i++){
        if(player_fds[i] != -1){
            close(player_fds[i]);
            player_fds[i] = -1;
        }
    }

    if(server_fd != -1){
        close(server_fd);
        server_fd = -1;
    }
    player_id = -1;
    printf("ネットワークを終了しました。\n");
}