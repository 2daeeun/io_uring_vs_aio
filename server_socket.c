// server_socket.c
#include "server_socket.h" // 헤더 파일 포함
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// 서버 소켓 생성 및 바인딩
int setup_server_socket() {
  int server_fd;
  struct sockaddr_in address;

  // 서버 소켓 생성
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    perror("socket 오류");
    exit(EXIT_FAILURE);
  }

  // 서버 주소 및 포트 설정
  memset(&address, 0, sizeof(struct sockaddr_in));
  address.sin_family = AF_INET; // 주소 체계를 IPv4로 사용
  address.sin_port = htons(PORT); // 포트 8080 사용 (바이트 순서로 변환)
  address.sin_addr.s_addr = INADDR_ANY; // 모든 장치에서 들어오는 IP 허용
  // address.sin_addr.s_addr = inet_addr("192.168.1.1"); // 특정 IP 주소 설정

  // 소켓에 주소 바인딩
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind 오류");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  // 연결 요청 대기
  if (listen(server_fd, 3) < 0) {
    perror("listen 오류");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  return server_fd;
}

// 클라이언트 연결 수락
int accept_client(int server_fd) {
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  int client_fd;

  // 클라이언트 연결 수락
  client_fd =
      accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
  if (client_fd < 0) {
    perror("accept 오류");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  return client_fd;
}
