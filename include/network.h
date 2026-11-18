
#ifndef NETWORK_H
#define NETWORK_H

/* サーバーへの接続 */
int Network_InitServer(int port);
int Network_Connect(const char *ip, int port);

/* 接続したプレイヤーのID */
int Network_GetPlayerID(void);

/* データの送受信 */
int Network_Send(const char *data);
int Network_Receive(char *buffer, int buffer_size);

/* 通信終了 */
void Network_Close(void);

#endif
