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
#include <limits.h>
#include "config.h"

#define SERVER0_PATH "./socket/server0_socket"
#define SERVER1_PATH "./socket/server1_socket"
#define BUFFER_SIZE 4096
#define NUM_CLIENT 4

/* 파일 병합 함수 */
void merge_files(int server_id) {
    int client_fds[NUM_CLIENT];  // 클라이언트 파일 디스크립터
    int current_values[NUM_CLIENT];  // 각 파일의 현재 값
    int valid_fd[NUM_CLIENT] = {1, 1, 1, 1};  // 파일이 아직 읽을 데이터가 있는지
    int active_files = NUM_CLIENT;  // 아직 처리해야 할 파일 수
    
    // 출력 파일 열기
    int output_fd;
    if (server_id == 0) {
        output_fd = open("bin/server/server0/server0.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    } else {
        output_fd = open("bin/server/server1/server1.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }

    if (output_fd < 0) {
        perror("출력 파일 열기 실패");
        return;
    }

    // 입력 파일들 열기
    for (int i = 0; i < NUM_CLIENT; i++) {
        char path[256];
        if (server_id == 0) {
            sprintf(path, "bin/server/server0/client%d.bin", i);
        } else {
            sprintf(path, "bin/server/server1/client%d.bin", i);
        }
        
        client_fds[i] = open(path, O_RDONLY);
        if (client_fds[i] < 0) {
            perror("입력 파일 열기 실패");
            valid_fd[i] = 0;
            active_files--;
            continue;
        }

        // 각 파일의 첫 값 읽기
        if (read(client_fds[i], &current_values[i], sizeof(int)) != sizeof(int)) {
            close(client_fds[i]);
            valid_fd[i] = 0;
            active_files--;
        }
    }

    // 모든 파일의 데이터를 비교하며 병합
    while (active_files > 0) {
        // 현재 가장 작은 값 찾기
        int min_value = INT_MAX;
        int min_idx = -1;

        for (int i = 0; i < NUM_CLIENT; i++) {
            if (valid_fd[i] && current_values[i] < min_value) {
                min_value = current_values[i];
                min_idx = i;
            }
        }

        if (min_idx != -1) {
            // 가장 작은 값 쓰기
            write(output_fd, &current_values[min_idx], sizeof(int));

            // 선택된 파일에서 다음 값 읽기
            if (read(client_fds[min_idx], &current_values[min_idx], sizeof(int)) != sizeof(int)) {
                close(client_fds[min_idx]);
                valid_fd[min_idx] = 0;
                active_files--;
            }
        }
    }

    // 파일 디스크립터 정리
    close(output_fd);
    for (int i = 0; i < NUM_CLIENT; i++) {
        if (valid_fd[i]) {
            close(client_fds[i]);
        }
    }
}

/* 서버i 프로세스 생성 함수 */
void server_process(const char *socket_path, int server_id) {
    int server_fd, client_fd;
    struct sockaddr_un server_addr;
    int connected_clients = 0;
    fd_set active_fd_set, read_fd_set;

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
        close(server_fd);
        exit(1);
    }

    // 소켓 대기
    if (listen(server_fd, NUM_CLIENT) < 0) {
        perror("소켓 대기 실패");
        close(server_fd);
        exit(1);
    }

    printf("서버 %d: %s 에서 대기 중...\n", server_id, socket_path);

    // fd_set 초기화
    FD_ZERO(&active_fd_set);
    FD_SET(server_fd, &active_fd_set);
    
    while (connected_clients < NUM_CLIENT) {
        read_fd_set = active_fd_set;
        
        if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
            perror("select 실패");
            exit(1);
        }

        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("클라이언트 연결 실패");
            continue;
        }

        printf("서버 %d: 클라이언트 %d 연결 수락\n", server_id, connected_clients);

        // 클라이언트 파일 저장
        int output_fd;
        if (server_id == 0) {
            if (connected_clients == 0) output_fd = open("bin/server/server0/client0.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            else if (connected_clients == 1) output_fd = open("bin/server/server0/client1.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            else if (connected_clients == 2) output_fd = open("bin/server/server0/client2.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            else output_fd = open("bin/server/server0/client3.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        } else {
            if (connected_clients == 0) output_fd = open("bin/server/server1/client0.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            else if (connected_clients == 1) output_fd = open("bin/server/server1/client1.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            else if (connected_clients == 2) output_fd = open("bin/server/server1/client2.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            else output_fd = open("bin/server/server1/client3.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }

        if (output_fd < 0) {
            perror("파일 생성 실패");
            close(client_fd);
            continue;
        }

        // 데이터 수신 및 파일 저장
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;
        while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
            if (write(output_fd, buffer, bytes_read) != bytes_read) {
                perror("파일 쓰기 실패");
                break;
            }
        }

        close(output_fd);
        close(client_fd);
        printf("서버 %d: 클라이언트 %d 데이터 저장 완료\n", server_id, connected_clients);
        connected_clients++;
    }

    // 모든 클라이언트 파일을 병합
    merge_files(server_id);

    close(server_fd);
    unlink(socket_path);
    printf("서버 %d: 모든 데이터 병합 완료\n", server_id);
    exit(0);
}

int main() {
    pid_t pid0, pid1;
    int status;

    // 서버0 생성
    if ((pid0 = fork()) == 0) {
        server_process(SERVER0_PATH, 0);
    }

    // 서버1 생성
    if ((pid1 = fork()) == 0) {
        server_process(SERVER1_PATH, 1);
    }

    // 부모 프로세스는 서버0과 서버1의 종료 대기
    waitpid(pid0, &status, 0);
    if (WIFEXITED(status)) {
        printf("서버 0이 정상 종료됨 (exit status: %d)\n", WEXITSTATUS(status));
    }

    waitpid(pid1, &status, 0);
    if (WIFEXITED(status)) {
        printf("서버 1이 정상 종료됨 (exit status: %d)\n", WEXITSTATUS(status));
    }

    printf("모든 서버 프로세스 종료\n");
    return 0;
}