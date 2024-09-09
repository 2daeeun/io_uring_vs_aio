// benchmark.c
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define REQUESTS 1000         // 총 요청 수
#define CONCURRENT_CLIENTS 50 // 동시 클라이언트 수

// 클라이언트 스레드 인자 구조체
struct client_args {
  const char *server_ip; // 서버의 IP 주소를 저장
  int num_requests; // 해당 스레드가 서버에 요청할 요청의 수
  double *results; // 각 요청의 결과 시간을 저장할 배열의 포인터
  int start_index; // 결과 배열에서 현재 스레드의 시작 인덱스 (결과 저장 위치)
};

// "Hello, World!" 메시지 배열 정의
static const char HELLO_WORLD[] = "Hello, World!";

static void *client_thread(void *args) {
  struct client_args *client_args = (struct client_args *)
      args; // 클라이언트 스레드 함수 인자로 받은 구조체를 타입 변환
  int socket_fd;                     // 소켓 파일 디스크립터
  struct sockaddr_in server_address; // 서버 주소 저장
  char buffer[BUFFER_SIZE];          // 응답 버퍼
  struct timeval start, end;
  double elapsed; // 전송 및 응답 시간을 저장할 변수

  for (int i = client_args->start_index; i < client_args->num_requests;
       i += CONCURRENT_CLIENTS) {
    // 소켓 생성
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
      perror("socket 오류");
      pthread_exit(NULL);
    }

    // 서버 주소 설정
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    if (inet_pton(AF_INET, client_args->server_ip, &server_address.sin_addr) <=
        0) {
      perror("inet_pton 오류");
      close(socket_fd);
      pthread_exit(NULL);
    }

    // 서버에 연결
    if (connect(socket_fd, (struct sockaddr *)&server_address,
                sizeof(server_address)) < 0) {
      perror("connect 오류");
      close(socket_fd);
      pthread_exit(NULL);
    }

    // "Hello, World!" 메시지 전송
    gettimeofday(&start, NULL);
    send(socket_fd, HELLO_WORLD, strlen(HELLO_WORLD), 0);

    // 서버로부터 응답 수신
    recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
    gettimeofday(&end, NULL);

    buffer[strlen(HELLO_WORLD)] = '\0'; // 응답 메시지 종료
    printf("Received: %s\n", buffer);

    // 시간 계산
    elapsed = (end.tv_sec - start.tv_sec) * 1000.0;    // sec to ms
    elapsed += (end.tv_usec - start.tv_usec) / 1000.0; // μs to ms

    client_args->results[i] = elapsed;

    close(socket_fd);
  }

  pthread_exit(NULL); // 스레드 종료
}

static void measure_performance(const char *server_executable,
                                const char *output_file) {
  pthread_t threads[CONCURRENT_CLIENTS]; // 클라이언트 스레드를 저장
  struct client_args client_args[CONCURRENT_CLIENTS]; // 각 클라이언트의 인자들
  double results[REQUESTS]; // 각 요청의 응답 시간 결과 저장
  struct timeval start, end; // 성능 측정의 시작과 종료 시간을 저장할 변수
  double total_time; // 전체 소요 시간 계산
  pid_t pid;         // 서버 프로세스의 PID를 저장할 변수

  // 서버 실행
  pid = fork();
  if (pid == 0) {
    // 자식 프로세스에서 서버 실행
    execl(server_executable, server_executable, NULL);
    perror("execl 오류");
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("fork 오류");
    exit(EXIT_FAILURE);
  }

  // 서버 준비 대기
  sleep(1);

  // 성능 측정을 위한 시간을 기록 (시작)
  gettimeofday(&start, NULL);

  for (int i = 0; i < CONCURRENT_CLIENTS; i++) {
    client_args[i].server_ip = "127.0.0.1"; // 서버 IP를 127.0.0.1로 고정
    client_args[i].num_requests = REQUESTS; // 클라이언트가 보낼 요청 수 설정
    client_args[i].results = results; // 결과 저장할 배열
    client_args[i].start_index = i;   // 시작 인덱스 설정

    // 스레드 생성
    pthread_create(&threads[i], NULL, client_thread, (void *)&client_args[i]);
  }

  // 스레드 종료 대기
  for (int i = 0; i < CONCURRENT_CLIENTS; i++) {
    pthread_join(threads[i], NULL);
  }

  // 성능 측정을 위한 시간을 기록 (종료)
  gettimeofday(&end, NULL);

  // 시간 계산
  total_time = (end.tv_sec - start.tv_sec) * 1000.0;    // sec to ms
  total_time += (end.tv_usec - start.tv_usec) / 1000.0; // μs to ms

  // 결과 파일 저장
  FILE *fp = fopen(output_file, "w");
  if (fp == NULL) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  fprintf(fp, "Total time: %.2f ms\n", total_time);

  for (int i = 0; i < REQUESTS; i++) {
    fprintf(fp, "Request %-4d: %.2f ms\n", i + 1, results[i]);
  }

  fclose(fp);

  // 서버 종료
  kill(pid, SIGTERM);    // 서버 종료 신호 보내기
  waitpid(pid, NULL, 0); // 서버 종료 대기
}

int main() {
  // server_aio 실행 및 성능 측정
  printf("┌─────────────────────────────────────────────┐\n");
  printf("│     server_aio에 대한 성능 테스트 실행      │\n");
  printf("└─────────────────────────────────────────────┘\n");
  measure_performance("./server_aio", "server_aio의_성능_결과.txt");

  // server_io_uring 실행 및 성능 측정
  printf("\n");
  printf("┌─────────────────────────────────────────────┐\n");
  printf("│   server_io_uring에 대한 성능 테스트 실행   │\n");
  printf("└─────────────────────────────────────────────┘\n");
  measure_performance("./server_io_uring", "server_io_uring의_성능_결과.txt");

  printf("\n성능 테스트 완료\n");
  printf("\"server_aio의_성능_결과.txt\"와 "
         "\"server_io_uring의_성능_결과.txt\"를 확인해주세요.\n");
  return 0;
}
