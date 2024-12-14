/*
* server
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "config.h"

#define SERVER0_PATH "./socket/server0_socket"
#define SERVER1_PATH "./socket/server1_socket"
#define BUFFER_SIZE 4096
#define NUM_CLIENT 4

/* 서버i 프로세스 생성 함수 */
void server_process(const char *socket_path, int server_id) {
    int server_fd, client_fd[NUM_CLIENT], max_fd, activity, i, valread;
    struct sockaddr_un server_addr;
    fd_set read_fds;
    int received_data[NUM_CLIENT][ROWS][COLS] = {0}; // 클라이언트별 수신 데이터
    int clients_received = 0;                        // 데이터 수신 완료한 클라이언트 수

    // 소켓 생성
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("소켓 생성 실패");
        exit(1);
    }

    // 소켓 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);

    // 기존 소켓 파일 삭제
    unlink(socket_path);

    // 소켓 바인드
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("소켓 바인드 실패");
        exit(1);
    }

    // 소켓 대기
    if (listen(server_fd, NUM_CLIENT) < 0) {
        perror("소켓 대기 실패");
        exit(1);
    }

    printf("서버 %d: %s 에서 대기 중...\n", server_id, socket_path);

    // 클라이언트 관리 배열 초기화
    for (i = 0; i < NUM_CLIENT; i++) {
        client_fd[i] = -1;
    }

    // 무한루프로 클라이언트에게 값 수신
    while (1) {
        if (clients_received == NUM_CLIENT) {
            printf("서버 %d: 모든 클라이언트 데이터 수신 완료\n", server_id);
            break;
        }

        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        max_fd = server_fd;

        // 클라이언트 소켓 추가
        for (i = 0; i < NUM_CLIENT; i++) {
            if (client_fd[i] > 0)
                FD_SET(client_fd[i], &read_fds);
            if (client_fd[i] > max_fd)
                max_fd = client_fd[i];
        }

        // select 호출
        activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select 실패");
            exit(1);
        }

        // 새 클라이언트 연결 처리
        if (FD_ISSET(server_fd, &read_fds)) { //서버에 이벤트가 발생헀는지 확인
            int new_socket = accept(server_fd, NULL, NULL);
            if (new_socket < 0) {
                perror("accept 실패");
                exit(1);
            }

            for (i = 0; i < NUM_CLIENT; i++) {
                if (client_fd[i] == -1) {
                    client_fd[i] = new_socket;
                    printf("서버 %d: 클라이언트 %d 등록 완료\n", server_id, i);
                    break;
                }
            }
        }

        // 기존 클라이언트 데이터 처리
        for (i = 0; i < NUM_CLIENT; i++) {
            if (client_fd[i] > 0 && FD_ISSET(client_fd[i], &read_fds)) {
                valread = read(client_fd[i], received_data[i], sizeof(received_data[i]));
                if (valread > 0) {
                    printf("서버 %d: 클라이언트 %d로부터 데이터 수신 완료\n", server_id, i);
                    clients_received++;
                    close(client_fd[i]);
                    client_fd[i] = -1;
                }
            }
        }
    } 
    //while루프 종료

    // 데이터 병합 및 저장
    char filename[30];
    snprintf(filename, sizeof(filename), "bin/server%d_data.bin", server_id);
    int output_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd < 0) {
        perror("파일 생성 실패");
        exit(1);
    }

    for (i = 0; i < NUM_CLIENT; i++) {
        if (write(output_fd, received_data[i], sizeof(received_data[i])) != sizeof(received_data[i])) {
            perror("파일 쓰기 실패");
            close(output_fd);
            exit(1);
        }
    }

    close(output_fd);
    printf("서버 %d: 데이터 병합 완료 -> %s\n", server_id, filename);

    close(server_fd);
    unlink(socket_path);
    exit(0);
}

int main() {
    int input_fd = open("bin/data.bin", O_RDONLY);
    if (input_fd < 0) {
        perror("bin/data.bin 파일을 열 수 없습니다");
        return 1;
    }

    int matrix[ROWS][COLS];
    int i, j;

    /* 파일에서 데이터 읽기 (저수준 입출력) */
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            if (read(input_fd, &matrix[i][j], sizeof(int)) != sizeof(int)) {
                perror("파일 읽기 오류");
                close(input_fd);
                return 1;
            }
        }
    }

    close(input_fd);

    int cols_per_client = COLS / OFFSET;

    /* 8개의 클라이언트 파일 생성, 분배 후 저장 */
    int client;
    for (client = 0; client < NUM_CLIENT; client++) {
        char filename[30];
        snprintf(filename, sizeof(filename), "bin/client/partition/sm%d.bin", client);

        int output_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (output_fd < 0) {
            perror("파일을 열 수 없습니다");
            return 1;
        }

        for (i = 0; i < ROWS; i++) {
            for (j = client * cols_per_client; j < (client + 1) * cols_per_client; j++) {
                if (write(output_fd, &matrix[i][j], sizeof(int)) != sizeof(int)) {
                    perror("파일 쓰기 오류");
                    close(output_fd);
                    return 1;
                }
            }
        }

        close(output_fd);
        printf("%s 파일이 생성되었습니다.\n", filename);
    }

    /* fork */
    pid_t pid0, pid1;

    // 서버0 생성
    if ((pid0 = fork()) == 0) {
        server_process(SERVER0_PATH, 0);
    }

    // 서버1 생성
    if ((pid1 = fork()) == 0) {
        server_process(SERVER1_PATH, 1);
    }

    // 부모 프로세스는 서버0과 서버1의 종료 대기
    waitpid(pid0, NULL, 0);
    waitpid(pid1, NULL, 0);
    printf("server종료\n");
}
