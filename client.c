#include <arpa/inet.h>
#include <fcntl.h>
#include <liburing.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define BUF_SIZE 4096

int main() {
  int client_fd;
  struct sockaddr_in server_addr;
  char buf[BUF_SIZE];

  client_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (client_fd < 0) {
    perror("socket");
    exit(1);
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
  server_addr.sin_port = htons(PORT);

  if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("connect");
    exit(1);
  }

  strcpy(buf, "Hello, server!");
  send(client_fd, buf, strlen(buf), 0);

  recv(client_fd, buf, BUF_SIZE, 0);
  printf("Received message from server: %s\n", buf);

  close(client_fd);

  return 0;
}
