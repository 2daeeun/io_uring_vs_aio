// server_io_uring.c
#include "server_socket.h"
#include <errno.h>
#include <liburing.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE                                                            \
  1024 // 읽기/ 쓰기 버퍼 크기 (예, 2048, 4096)
       // 버퍼 크기가 작으면 오버헤드 증가
#define QUEUE_DEPTH                                                            \
  1 // 동시에 처리할 수 있는 I/O 요청의 수를 정의 (예, 4, 8, 16)
    // QUEUE_DEPTH 크기를 늘리면 I/O 처리랑은 늘릴수 있지만
    // 병렬처리 리소스가 증가

// 클라이언트 요청을 처리하는 함수
void handle_client(int client_fd) {
  struct io_uring ring;
  struct io_uring_cqe *cqe;
  struct io_uring_sqe *sqe;
  char buffer[BUFFER_SIZE]; // 데이터 입출력을 위한 버퍼
  ssize_t len; // I/O 작업에서 읽거나 쓸 데이터의 길이
               // - ssize_t는 부호가 있는 타입, -1 반환 가능
               // - size_t는 부호가 없는 타입

  // io_uring 초기화
  if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0) {
    perror("io_uring_queue_init 오류");
    close(client_fd);
    return;
  }

  while (1) {
    // 읽기 요청 준비
    sqe = io_uring_get_sqe(&ring); // SQ에서 SQE를 가져온다.
    if (!sqe) {
      perror("읽기 요청의 io_uring_get_sqe 오류");
      break;
    }

    // 비동기 읽기 작업을 SQE에 설정한다.
    io_uring_prep_read(sqe, client_fd, buffer, BUFFER_SIZE, 0);

    // 읽기 요청 제출
    // io_uring_submit : SQE에 있는 I/O 요청들을 커널로 전송한다.
    //                   (SQ에 있는 I/O 요청을 커널로 전송한다.)
    if (io_uring_submit(&ring) < 0) {
      perror("읽기 요청의 io_uring_submit 오류");
      break;
    }

    // 요청 완료 대기
    // io_uring_wait_cqe : CQ에서 완료된 I/O 요청을 기다린다.
    if (io_uring_wait_cqe(&ring, &cqe) < 0) {
      perror("읽기 요청의 io_uring_wait_cqe 오류");
      break;
    }

    // cqe->res는 CQE에서 I/O 요청의 결과를 담고 있는 필드이다.
    // - 성공적인 경우 실제 읽기/쓰기된 바이트 수를 포함(0 이상),
    // - 오류 발생 시 음수 값
    len = cqe->res;
    io_uring_cqe_seen(&ring, cqe); // CQE를 처리 완료로 표시

    if (len <= 0) {
      break; // 연결 종료 또는 오류 발생
    }

    buffer[len] = '\0';
    printf("Received: %s\n", buffer);

    // 쓰기 요청 준비
    sqe = io_uring_get_sqe(&ring);
    if (!sqe) {
      perror("쓰기 요청의 io_uring_get_sqe 오류");
      break;
    }

    // 비동기 쓰기 작업을 SQE에 설정한다.
    io_uring_prep_write(sqe, client_fd, buffer, len, 0);

    // 쓰기 요청 제출
    if (io_uring_submit(&ring) < 0) {
      perror("쓰기 요청의 io_uring_submit 오류");
      break;
    }

    // 요청 완료 대기
    if (io_uring_wait_cqe(&ring, &cqe) < 0) {
      perror("쓰기 요청의 io_uring_wait_cqe 오류");
      break;
    }
    io_uring_cqe_seen(&ring, cqe); // CQE를 처리 완료로 표시
  }

  // io_uring의 SQ와 CQ를 해제
  io_uring_queue_exit(&ring);

  // 클라이언트 소켓 닫기
  close(client_fd);
}

int main() {
  int server_fd, client_fd;

  // 서버 소켓 설정
  server_fd = setup_server_socket();

  while (1) {
    // 클라이언트 연결 수락
    client_fd = accept_client(server_fd);

    // 클라이언트 처리
    handle_client(client_fd);
  }

  // 서버 소켓 닫기
  close(server_fd);
  return 0;
}
