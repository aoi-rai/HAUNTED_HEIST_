#ifndef NETWORK_H
#define NETWORK_H

/*ネットワークの初期化*/
int Network_InitServer(int port);
int Network_Connect(const char *ip, int port);

/*データの送受信*/
int Network_Send(const char *data);
int Network_Receive(char *buffer, int buffer_size);

/*通信終了*/
void Network_Close(void);

#endif