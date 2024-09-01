#include "server_socket.h"
#include <liburing.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define QUEUE_DEPTH 256
#define BUF_SIZE 4096

struct io_data {
  int fd;             // 클라이언트 파일 디스크럽터
  char buf[BUF_SIZE]; // 데이터 버퍼
};

// 새로운 연결을 수락하기 위한 요청 추가 함수
void add_accept_request(struct io_uring *ring, int server_fd) {
  struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
  io_uring_prep_accept(sqe, server_fd, NULL, NULL, 0);
}

// 읽기 요청을 추가하는 함수
void add_read_request(struct io_uring *ring, int client_fd,
                      struct io_data *data) {
  struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
  io_uring_prep_read(sqe, client_fd, data->buf, BUF_SIZE, 0);
  sqe->user_data = (unsigned long)data;
}

// 쓰기 요청을 추가하는 함수
void add_write_request(struct io_uring *ring, struct io_data *data,
                       ssize_t len) {
  struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
  io_uring_prep_write(sqe, data->fd, data->buf, len, 0);
  sqe->user_data = (unsigned long)data;
}

int main() {
  int server_fd = setup_server_socket();
  struct io_uring ring;
  io_uring_queue_init(QUEUE_DEPTH, &ring, 0); // io_uring 큐 초기화

  add_accept_request(&ring, server_fd); // 최초 연결 요청 추가

  while (1) {
    struct io_uring_cqe *cqe;
    io_uring_submit(&ring);         // 큐에 요청 제출
    io_uring_wait_cqe(&ring, &cqe); // 완료 이벤트 대기

    struct io_data *data = (struct io_data *)cqe->user_data;

    if (cqe->res >= 0) {
      if (cqe->res == 0) {
        // 새로운 연결 수락
        int client_fd = cqe->res;
        struct io_data *client_data = malloc(sizeof(*client_data));
        client_data->fd = client_fd;
        add_read_request(&ring, client_fd, client_data); // 읽기 요청을 추가
      } else if (data->fd > 0) {
        // 읽기 또는 쓰기 처리
        if (cqe->res > 0) {
          add_write_request(&ring, data, cqe->res);
        } else {
          close(data->fd);
          free(data);
        }
      }
    } else {
      perror("io_uring 작업 오류");
    }
    io_uring_cqe_seen(&ring, cqe);
  }
  io_uring_queue_exit(&ring);
  close(server_fd);

  return EXIT_SUCCESS;
}
