#include <fcntl.h>
#include <liburing.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 4096

int main() {
  struct io_uring ring;
  struct io_uring_sqe *sqe;
  struct io_uring_cqe *cqe;
  int fd, ret;
  char buf[BUF_SIZE];

  // io_uring 초기화
  if (io_uring_queue_init(16, &ring, 0) < 0) {
    perror("io_uring_queue_init");
    exit(1);
  }

  // 읽을 파일 열기
  fd = open("example.txt", O_RDONLY);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  // io_uring 작업 설정
  sqe = io_uring_get_sqe(&ring);
  if (!sqe) {
    perror("io_uring_get_sqe");
    exit(1);
  }

  io_uring_prep_read(sqe, fd, buf, BUF_SIZE, 0);

  // io_uring 작업 제출
  io_uring_submit(&ring);

  // io_uring 작업 결과 확인
  ret = io_uring_wait_cqe(&ring, &cqe);
  if (ret < 0) {
    perror("io_uring_wait_cqe");
    exit(1);
  }

  // 결과 처리
  if (cqe->res < 0) {
    perror("io_uring");
    exit(1);
  }

  // 결과 출력
  printf("Read %d bytes: %s\n", cqe->res, buf);

  // 파일과 io_uring 정리
  close(fd);
  io_uring_queue_exit(&ring);

  return 0;
}
