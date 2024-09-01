#include <fcntl.h>
#include <liburing.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 12345
#define BUF_SIZE 4096

int main() {
  struct io_uring ring;
  struct io_uring_sqe *sqe_recv, *sqe_send;
  struct io_uring_cqe *cqe;
  int server_fd, client_fd, ret;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_addr_len;
  char buf[BUF_SIZE];

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket");
    exit(1);
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("bind");
    exit(1);
  }

  if (listen(server_fd, 5) < 0) {
    perror("listen");
    exit(1);
  }

  if (io_uring_queue_init(16, &ring, 0) < 0) {
    perror("io_uring_queue_init");
    exit(1);
  }

  client_addr_len = sizeof(client_addr);
  client_fd =
      accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
  if (client_fd < 0) {
    perror("accept");
    exit(1);
  }

  sqe_recv = io_uring_get_sqe(&ring);
  if (!sqe_recv) {
    perror("io_uring_get_sqe for recv");
    exit(1);
  }

  io_uring_prep_recv(sqe_recv, client_fd, buf, BUF_SIZE, 0);

  io_uring_submit(&ring);

  ret = io_uring_wait_cqe(&ring, &cqe);
  if (ret < 0) {
    perror("io_uring_wait_cqe");
    exit(1);
  }

  sqe_send = io_uring_get_sqe(&ring);
  if (!sqe_send) {
    perror("io_uring_get_sqe for send");
    exit(1);
  }

  io_uring_prep_send(sqe_send, client_fd, buf, cqe->res, 0);

  io_uring_submit(&ring);

  ret = io_uring_wait_cqe(&ring, &cqe);
  if (ret < 0) {
    perror("io_uring_wait_cqe");
    exit(1);
  }

  close(client_fd);
  io_uring_queue_exit(&ring);

  return 0;
}
