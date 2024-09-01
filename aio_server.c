#include "server_socket.h"
#include <aio.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct aio_data {
  struct aiocb cb;    // 비동기 I/O 제어 블록
  char buf[BUF_SIZE]; // 데이터 버퍼
  int fd;             // 클라이언트 파일 디스크립터
};

// 읽기 요청을 추가하는 함수
void add_read_request(int client_fd, struct aio_data *data) {
  memset(&data->cb, 0, sizeof(struct aiocb));
  data->cb.aio_fildes = client_fd; // 파일 디스크립터 설정
  data->cb.aio_buf = data->buf;    // 버퍼 설정
  data->cb.aio_nbytes = BUF_SIZE;  // 읽기 요청할 바이트 수 설정
  aio_read(&data->cb);             // 비동기 읽기 요청
}

// 쓰기 요청을 추가하는 함수
void add_write_request(struct aio_data *data, ssize_t len) {
  memset(&data->cb, 0, sizeof(struct aiocb));
  data->cb.aio_fildes = data->fd; // 파일 디스크립터 설정
  data->cb.aio_buf = data->buf;   // 버퍼 설정
  data->cb.aio_nbytes = len;      // 쓰기 요청할 바이트 수 설정
  aio_write(&data->cb);           // 비동기 쓰기 요청
}

int main() {
  int server_fd = setup_server_socket(); // 서버 소켓 설정

  while (1) {
    int client_fd = accept(server_fd, NULL, NULL); // 클라이언트 연결 수락

    if (client_fd < 0) {
      perror("accept() 오류");
      continue;
    }

    struct aio_data *data = malloc((sizeof(struct aio_data)));
    data->fd = client_fd;
    add_read_request(client_fd, data); // 읽기 요청 추가

    // 읽기 요청 완료 대기
    while (aio_error(&data->cb) == EINPROGRESS) {
      /* 비동기 I/O작업을 수행하거나, 그냥 대기
       *
       * EINPROGRESS는 비동기 작업이 아직 완료되지 않았음을 나타내는 오류
       * 코드이다.
       * (※ man page의 connect 참고하기) */
    }

    // 읽기 요청 완료 후 데이터가 있는 경우
    if (aio_return(&data->cb) > 0) {
      add_write_request(data,
                        aio_return(&data->cb)); // 읽은 데이터 쓰기 요청 추가

      // 쓰기 요청 완료 대기
      while (aio_error(&data->cb) == EINPROGRESS) {
      }
    }
    close(client_fd);
    free(data);
  }

  close(server_fd);
  return EXIT_SUCCESS;
}
