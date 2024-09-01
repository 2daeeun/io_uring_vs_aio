#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#define PORT 8080
#define BACKLOG 128

/* 버퍼 사이즈를 4096으로 할지 8192로 할지 고민하기
 * */
#define BUF_SIZE 4096

int setup_server_socket();

#endif
