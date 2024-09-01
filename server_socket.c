#include "server_socket.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/* 소켓을 설정하는 함수입니다.
 * */
int setup_server_socket() {
  int server_fd;
  struct sockaddr_in address;

  // 소켓 생성 (TCP 소켓)
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket() 오류");
    exit(EXIT_FAILURE);
  }

  // 소켓 SO_REUSEADDR 옵션 설정
  int opt = 1; // SO_REUSEADDR를 True로 설정하기 위해 사용
  if ((setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) <
      0) {
    perror("setsockopt(SO_REUSEADDR) 오류");
    exit(EXIT_FAILURE);
  }

  // 소켓 SO_REUSEADDR 옵션 설정 (선택사항)
  // if ((setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) <
  //     0) {
  //   perror("setsockopt(SO_REUSEPORT) 오류");
  //   exit(EXIT_FAILURE);
  // }

  // 주소와 포트 설정
  memset(&address, 0, sizeof(struct sockaddr_in));
  address.sin_family = AF_INET; // 주소 체계를 IPv4로 사용
  address.sin_port = htons(PORT); // 포트 8080 사용 (바이트 순서로 변환)
  address.sin_addr.s_addr = INADDR_ANY; // 모든 장치에서 들어오는 IP 허용
  // address.sin_addr.s_addr = inet_addr("192.168.1.1");  // 특정 IP 주소 설정

  // 소켓에 주소 바인딩
  if ((bind(server_fd, (struct sockaddr *)&address, sizeof(address))) < 0) {
    perror("bind() 오류");
    exit(EXIT_FAILURE);
  }

  // 연결 요청 대기
  if (listen(server_fd, BACKLOG)) {
    perror("listen() 오류");
    exit(EXIT_FAILURE);
  }

  return server_fd;
}
