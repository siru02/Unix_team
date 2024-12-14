#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "config.h"

#define MSG_KEY 5678 /* 메시지 큐 키 */
#define BUFFER_SIZE 4096
#define SERVER0_PATH "./socket/server0_socket"
#define SERVER1_PATH "./socket/server1_socket"

struct message {
    long msg_type;
    int sender_id; /* 송신자 ID */
    int data[ROWS / NUM_CLIENTS][COLS / NUM_CLIENTS]; /* 16x16 크기 데이터 */
};

/* 서버에 데이터를 전송하는 코드 */
void send_file_to_server(const char *file_path, int client_num) { //클라이언트 파일의 경로
    int client_fd;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];
    int file_fd, bytes_read;

    // 소켓 생성
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("소켓 생성 실패");
        exit(1);
    }

    // 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    if (client_num == 0 || client_num == 1 || client_num == 4 || client_num == 5)
        strncpy(server_addr.sun_path, SERVER0_PATH, sizeof(server_addr.sun_path) - 1);
    else // 2 3 6 7
        strncpy(server_addr.sun_path, SERVER1_PATH, sizeof(server_addr.sun_path) - 1);

    // 서버 연결
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("서버 연결 실패");
        close(client_fd);
        exit(1);
    }

    // 파일 열기
    file_fd = open(file_path, O_RDONLY);
    if (file_fd < 0) {
        perror("파일 열기 실패");
        close(client_fd);
        exit(1);
    }

    // 파일 내용 전송
    while ((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
        if (write(client_fd, buffer, bytes_read) != bytes_read) { // 파일에서 읽은걸 소켓에 작성
            perror("데이터 전송 실패");
            close(file_fd);
            close(client_fd);
            exit(1);
        }
    }

    close(file_fd);
    close(client_fd);
    printf("클라이언트: 파일 %s %s서버로 전송 완료\n", file_path, server_addr.sun_path);
}

int main() {
    pid_t pids[NUM_CLIENTS];
    int msgid;
    int row, col, i, src, dest;

    /* 메시지 큐 생성 */
    // msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    // if (msgid < 0) {
    //     perror("메시지 큐 생성 실패");
    //     exit(1);
    // }

    /* 반복문 순회하면서 클라이언트 생성 */
    for (i = 0; i < NUM_CLIENTS; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork 실패");
            exit(1);
        } 
        else if (pids[i] == 0) {
            /* 클라이언트 프로세스 */
            // char path[40];
            // snprintf(path, sizeof(path), "bin/client/partition/sm%d.bin", i); // 파티션작업

            // int input_fd = open(path, O_RDONLY);
            // if (input_fd < 0) {
            //     perror("bin/client/partition/sm%d.bin : 파일 열기 실패");
            //     exit(1);
            // }

            /* 128행 × 16열 데이터 읽기 */
            // int matrix[ROWS][COLS / NUM_CLIENTS]; 
            // for (row = 0; row < ROWS; row++) {
            //     if (read(input_fd, matrix[row], sizeof(int) * (COLS / NUM_CLIENTS)) 
            //         != sizeof(int) * (COLS / NUM_CLIENTS)) {
            //         perror("bin/client/partition/sm%d.bin : 파일 읽기 오류");
            //         close(input_fd);
            //         exit(1);
            //     }
            // }
            // close(input_fd);

            /* 메시지 송신: dest에 따라 행 블록을 선택 */
            // for (dest = 0; dest < NUM_CLIENTS; dest++) {
            //     struct message msg;
            //     msg.msg_type = dest + 1;
            //     msg.sender_id = i;

            //     int start_row = dest * (ROWS / NUM_CLIENTS); // dest에 해당하는 16행 시작점
            //     for (row = 0; row < ROWS / NUM_CLIENTS; row++) {
            //         for (col = 0; col < COLS / NUM_CLIENTS; col++) {
            //             /* start_row부터 16행, 각 행당 16열 블록 전송 */
            //             msg.data[row][col] = matrix[start_row + row][col];
            //         }
            //     }

            //     if (msgsnd(msgid, &msg, sizeof(msg.data) + sizeof(msg.sender_id), 0) == -1) {
            //         perror("메시지 전송 실패");
            //         exit(1);
            //     }

            //     printf("Client %d: 메시지 전송 완료 (to Client %d), rows [%d~%d], cols[0~15]\n",
            //            i, dest, start_row, start_row + (ROWS / NUM_CLIENTS) - 1);
            // }

            /* 메시지 수신 및 재배치 */
            //struct message received_msg;
            
	    /* local_data[][]에 수신 후 순서대로 저장 */
	    // int local_data[ROWS / NUM_CLIENTS][COLS];
        //     memset(local_data, 0, sizeof(local_data));

        //     for (src = 0; src < NUM_CLIENTS; src++) {
        //         if (msgrcv(msgid, &received_msg, sizeof(received_msg.data) + sizeof(received_msg.sender_id),
        //                    i + 1, 0) == -1) {
        //             perror("메시지 수신 실패");
        //             exit(1);
        //         }

        //         int sender = received_msg.sender_id;
        //         printf("Client %d: 메시지 수신 완료 (from Client %d)\n", i, sender);

        //         for (row = 0; row < ROWS / NUM_CLIENTS; row++) {
        //             for (col = 0; col < COLS / NUM_CLIENTS; col++) {
        //                 local_data[row][col + sender * (COLS / NUM_CLIENTS)] = received_msg.data[row][col];
        //             }
        //         }
        //     }

            /* 최종 결과 저장 */
            // snprintf(path, sizeof(path), "bin/client/collected/sm%d.bin", i);
            // int output_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            // if (output_fd < 0) {
            //     perror("bin/client/collected/sm%d.bin : 파일 열기 실패");
            //     exit(1);
            // }

            // for (row = 0; row < ROWS / NUM_CLIENTS; row++) {
            //     if (write(output_fd, local_data[row], sizeof(int) * COLS) != sizeof(int) * COLS) {
            //         perror("bin/client/collected/sm%d.bin : 파일 쓰기 실패");
            //         close(output_fd);
            //         exit(1);
            //     }
            // }

            // close(output_fd);
            // printf("%s 파일이 생성되었습니다.\n", path);
            // printf("client %d forked (pid:%d)\n", i, getpid());

            // socket으로 서버에 데이터를 전송해야한다
            char path[40];
            snprintf(path, sizeof(path), "bin/client/collected/sm%d.bin", i);
            printf("client %d send to server\n", i);
            send_file_to_server(path, i);

            exit(0);
        }
    }

    for (i = 0; i < NUM_CLIENTS; i++) {
        wait(NULL);
    }

    /* 메시지 큐 삭제 */
    // if (msgctl(msgid, IPC_RMID, NULL) == -1) {
    //     perror("메시지 큐 삭제 실패");
    //     exit(1);
    // }

    printf("\n\n모든 클라이언트 프로세스가 종료되었습니다.\n");
    return 0;
}

