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
#define MAX_ELEMENTS (ROWS * COLS / NUM_CLIENTS)

/* 정렬을 위한 비교 함수 */
int compare(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

/* 서버i 프로세스 생성 함수 */
void server_process(const char *socket_path, int server_id) {
    int server_fd, client_fd;
    struct sockaddr_un server_addr;
    int connected_clients = 0;
    fd_set active_fd_set, read_fd_set;
    int *all_data = NULL;
    int total_elements = 0;

    // 데이터 저장을 위한 메모리 할당
    all_data = (int *)malloc(MAX_ELEMENTS * NUM_CLIENT * sizeof(int));
    if (all_data == NULL) {
        perror("메모리 할당 실패");
        exit(1);
    }

    // 소켓 생성
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("소켓 생성 실패");
        free(all_data);
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
        free(all_data);
        close(server_fd);
        exit(1);
    }

    // 소켓 대기
    if (listen(server_fd, NUM_CLIENT) < 0) {
        perror("소켓 대기 실패");
        free(all_data);
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
            free(all_data);
            exit(1);
        }

        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("클라이언트 연결 실패");
            continue;
        }

        printf("서버 %d: 클라이언트 %d 연결 수락\n", server_id, connected_clients);

        // 임시 파일 경로 구성 및 생성
        char client_path[256];
        if (server_id == 0) {
            if (connected_clients == 0) {
                int output_fd = open("bin/server/server0/client0.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (output_fd < 0) {
                    perror("파일 생성 실패");
                    close(client_fd);
                    continue;
                }
                // 데이터 수신 및 파일 저장
                int buffer[BUFFER_SIZE/sizeof(int)];
                ssize_t bytes_read;

                while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
                    int nums = bytes_read / sizeof(int);
                    memcpy(all_data + total_elements, buffer, bytes_read);
                    total_elements += nums;
                    
                    if (write(output_fd, buffer, bytes_read) != bytes_read) {
                        perror("파일 쓰기 실패");
                        break;
                    }
                }
                close(output_fd);
            }
            else if (connected_clients == 1) {
                int output_fd = open("bin/server/server0/client1.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (output_fd < 0) {
                    perror("파일 생성 실패");
                    close(client_fd);
                    continue;
                }
                // 데이터 수신 및 파일 저장
                int buffer[BUFFER_SIZE/sizeof(int)];
                ssize_t bytes_read;

                while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
                    int nums = bytes_read / sizeof(int);
                    memcpy(all_data + total_elements, buffer, bytes_read);
                    total_elements += nums;
                    
                    if (write(output_fd, buffer, bytes_read) != bytes_read) {
                        perror("파일 쓰기 실패");
                        break;
                    }
                }
                close(output_fd);
            }
            else if (connected_clients == 2) {
                int output_fd = open("bin/server/server0/client2.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (output_fd < 0) {
                    perror("파일 생성 실패");
                    close(client_fd);
                    continue;
                }
                // 데이터 수신 및 파일 저장
                int buffer[BUFFER_SIZE/sizeof(int)];
                ssize_t bytes_read;

                while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
                    int nums = bytes_read / sizeof(int);
                    memcpy(all_data + total_elements, buffer, bytes_read);
                    total_elements += nums;
                    
                    if (write(output_fd, buffer, bytes_read) != bytes_read) {
                        perror("파일 쓰기 실패");
                        break;
                    }
                }
                close(output_fd);
            }
            else {
                int output_fd = open("bin/server/server0/client3.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (output_fd < 0) {
                    perror("파일 생성 실패");
                    close(client_fd);
                    continue;
                }
                // 데이터 수신 및 파일 저장
                int buffer[BUFFER_SIZE/sizeof(int)];
                ssize_t bytes_read;

                while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
                    int nums = bytes_read / sizeof(int);
                    memcpy(all_data + total_elements, buffer, bytes_read);
                    total_elements += nums;
                    
                    if (write(output_fd, buffer, bytes_read) != bytes_read) {
                        perror("파일 쓰기 실패");
                        break;
                    }
                }
                close(output_fd);
            }
        }
        else {
            if (connected_clients == 0) {
                int output_fd = open("bin/server/server1/client0.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (output_fd < 0) {
                    perror("파일 생성 실패");
                    close(client_fd);
                    continue;
                }
                // 데이터 수신 및 파일 저장
                int buffer[BUFFER_SIZE/sizeof(int)];
                ssize_t bytes_read;

                while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
                    int nums = bytes_read / sizeof(int);
                    memcpy(all_data + total_elements, buffer, bytes_read);
                    total_elements += nums;
                    
                    if (write(output_fd, buffer, bytes_read) != bytes_read) {
                        perror("파일 쓰기 실패");
                        break;
                    }
                }
                close(output_fd);
            }
            else if (connected_clients == 1) {
                int output_fd = open("bin/server/server1/client1.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (output_fd < 0) {
                    perror("파일 생성 실패");
                    close(client_fd);
                    continue;
                }
                // 데이터 수신 및 파일 저장
                int buffer[BUFFER_SIZE/sizeof(int)];
                ssize_t bytes_read;

                while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
                    int nums = bytes_read / sizeof(int);
                    memcpy(all_data + total_elements, buffer, bytes_read);
                    total_elements += nums;
                    
                    if (write(output_fd, buffer, bytes_read) != bytes_read) {
                        perror("파일 쓰기 실패");
                        break;
                    }
                }
                close(output_fd);
            }
            else if (connected_clients == 2) {
                int output_fd = open("bin/server/server1/client2.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (output_fd < 0) {
                    perror("파일 생성 실패");
                    close(client_fd);
                    continue;
                }
                // 데이터 수신 및 파일 저장
                int buffer[BUFFER_SIZE/sizeof(int)];
                ssize_t bytes_read;

                while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
                    int nums = bytes_read / sizeof(int);
                    memcpy(all_data + total_elements, buffer, bytes_read);
                    total_elements += nums;
                    
                    if (write(output_fd, buffer, bytes_read) != bytes_read) {
                        perror("파일 쓰기 실패");
                        break;
                    }
                }
                close(output_fd);
            }
            else {
                int output_fd = open("bin/server/server1/client3.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (output_fd < 0) {
                    perror("파일 생성 실패");
                    close(client_fd);
                    continue;
                }
                // 데이터 수신 및 파일 저장
                int buffer[BUFFER_SIZE/sizeof(int)];
                ssize_t bytes_read;

                while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
                    int nums = bytes_read / sizeof(int);
                    memcpy(all_data + total_elements, buffer, bytes_read);
                    total_elements += nums;
                    
                    if (write(output_fd, buffer, bytes_read) != bytes_read) {
                        perror("파일 쓰기 실패");
                        break;
                    }
                }
                close(output_fd);
            }
        }

        close(client_fd);
        printf("서버 %d: 클라이언트 %d 데이터 저장 완료\n", server_id, connected_clients);
        connected_clients++;
    }

    // 모든 데이터 정렬
    qsort(all_data, total_elements, sizeof(int), compare);

    // 정렬된 데이터를 단일 파일에 저장
    int final_fd;
    if (server_id == 0) {
        final_fd = open("bin/server/server0/server0.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    } else {
        final_fd = open("bin/server/server1/server1.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }

    if (final_fd < 0) {
        perror("최종 파일 생성 실패");
        free(all_data);
        close(server_fd);
        exit(1);
    }

    // 정렬된 데이터 쓰기
    if (write(final_fd, all_data, total_elements * sizeof(int)) != total_elements * sizeof(int)) {
        perror("최종 파일 쓰기 실패");
    }

    close(final_fd);
    close(server_fd);
    unlink(socket_path);
    free(all_data);

    printf("서버 %d: 모든 데이터 정렬 및 저장 완료\n", server_id);
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