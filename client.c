// client.c
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
  int socket_fd;
  struct sockaddr_in server_address;
  char buffer[BUFFER_SIZE] = "Hello, World!";
  ssize_t n;

  // 소켓 생성
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    perror("socket 오류");
    exit(EXIT_FAILURE);
  }

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(PORT);

  // 서버에 연결
  if (connect(socket_fd, (struct sockaddr *)&server_address,
              sizeof(server_address)) < 0) {
    perror("connection 오류");
    close(socket_fd);
    exit(EXIT_FAILURE);
  }

  // 'Hello, World!' 메시지 전송
  send(socket_fd, buffer, strlen(buffer), 0);
  printf("전송: %s\n", buffer);

  // 서버로부터 응답 수신
  n = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
  if (n <= 0) {
    perror("recv 오류");
  } else {
    buffer[n] = '\0';
    printf("Received: %s\n", buffer);
  }

  close(socket_fd);
  return 0;
}
