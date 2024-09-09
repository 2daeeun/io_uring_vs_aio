// server_socket.h
#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#define PORT 8080

// 서버 소켓 생성 및 바인딩
int setup_server_socket();

// 클라이언트 연결 수락
int accept_client(int server_fd);

#endif // SERVER_SOCKET_H
