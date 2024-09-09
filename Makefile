# 컴파일러 설정
CC = gcc

# 플래그 설정
# -00: 최적화를 하지 않음
# -01: 기본 수준의 최적화
# -02: 중간 수준의 최적화
# -03: 최대 수준의 최적화
CFLAGS = -Wall -Wextra -O3

# 라이브러리 설정
# -lrt: POSIX(AIO 포함) 라이브러리
# -luring: liburing 라이브러리
# -lpthread: POSIX 스레드 라이브러리
LDFLAGS_AIO = -lrt
LDFLAGS_IOURING = -luring
LDFLAGS_PTHREAD = -lpthread

# 소스 파일
SRCS_AIO = server_aio.c server_socket.c
SRCS_IOURING = server_io_uring.c server_socket.c
SRCS_BENCH = benchmark.c
SRCS_CLIENT = client.c

# 목적 파일 (build 디렉토리에 생성)
OBJS_AIO = $(patsubst %.c, build/%.o, $(SRCS_AIO))
OBJS_IOURING = $(patsubst %.c, build/%.o, $(SRCS_IOURING))
OBJS_BENCH = $(patsubst %.c, build/%.o, $(SRCS_BENCH))
OBJS_CLIENT = $(patsubst %.c, build/%.o, $(SRCS_CLIENT))

# 실행 파일 (build 디렉토리에 생성)
EXEC_AIO = build/server_aio
EXEC_IOURING = build/server_io_uring
EXEC_BENCH = build/benchmark
EXEC_CLIENT = build/client

# 모든 파일을 빌드
all: build_dir $(EXEC_AIO) $(EXEC_IOURING) $(EXEC_BENCH) $(EXEC_CLIENT)

# build 디렉토리 생성 규칙
build_dir:
	mkdir -p build

# server_aio 파일 생성 규칙
$(EXEC_AIO): $(OBJS_AIO)
	$(CC) $(CFLAGS) -o $@ $(OBJS_AIO) $(LDFLAGS_AIO)

# server_io_uring 파일 생성 규칙
$(EXEC_IOURING): $(OBJS_IOURING)
	$(CC) $(CFLAGS) -o $@ $(OBJS_IOURING) $(LDFLAGS_IOURING)

# benchmark 파일 생성 규칙
$(EXEC_BENCH): $(OBJS_BENCH)
	$(CC) $(CFLAGS) -o $@ $(OBJS_BENCH) $(LDFLAGS_PTHREAD)

# client 파일 생성 규칙
$(EXEC_CLIENT): $(OBJS_CLIENT)
	$(CC) $(CFLAGS) -o $@ $(OBJS_CLIENT)

# 목적 파일 생성 규칙
build/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# clean 규칙 (build 디렉토리 삭제)
clean:
	rm -rf build

# .PHONY
.PHONY: all clean build_dir
