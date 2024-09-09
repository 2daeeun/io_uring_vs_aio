// server_aio.c
#include "server_socket.h"
#include <aio.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE                                                            \
  1024 // 읽기/ 쓰기 버퍼 크기 (예, 2048, 4096)
       // 버퍼 크기가 작으면 오버헤드 증가

// 클라이언트 요청을 처리하는 함수
void handle_client(int client_fd) {
  char buffer[BUFFER_SIZE];
  struct aiocb read_req, write_req; // I/O 제어 블록
  ssize_t len; // I/O 작업에서 읽거나 쓸 데이터의 길이
               // - ssize_t는 부호가 있는 타입, -1 반환 가능
               // - size_t는 부호가 없는 타입

  while (1) {
    // 읽기 요청 설정
    memset(&read_req, 0, sizeof(struct aiocb));
    read_req.aio_fildes = client_fd;
    read_req.aio_buf = buffer;
    read_req.aio_nbytes = BUFFER_SIZE;
    read_req.aio_offset = 0;

    // 읽기 요청 전송
    if (aio_read(&read_req) == -1) {
      perror("aio_read 오류");
      break;
    }

    // 읽기 요청 완료 대기
    while (aio_error(&read_req) == EINPROGRESS) {
      // 읽기 요청이 완료될 때까지 무한 대기
    }

    // 읽기 결과 가져오기
    len = aio_return(&read_req);
    if (len <= 0) {
      break; // 연결 종료 또는 오류 발생
    }

    buffer[len] = '\0';
    printf("Received: %s\n", buffer);

    // 쓰기 요청 설정
    memset(&write_req, 0, sizeof(struct aiocb));
    write_req.aio_fildes = client_fd;
    write_req.aio_buf = buffer;
    write_req.aio_nbytes = len;
    write_req.aio_offset = 0;

    // 쓰기 요청 전송
    if (aio_write(&write_req) == -1) {
      perror("aio_write 오류");
      break;
    }

    // 쓰기 요청 완료 대기
    while (aio_error(&write_req) == EINPROGRESS) {
      // 쓰기 요청이 완료될 때까지 무한 대기
    }
  }

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
