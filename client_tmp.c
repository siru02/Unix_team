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

#define MSG_KEY 5678 /* �޽��� ť Ű */
#define BUFFER_SIZE 4096
#define SERVER0_PATH "./socket/server0_socket"
#define SERVER1_PATH "./socket/server1_socket"

struct message {
    long msg_type;
    int sender_id; /* �۽��� ID */
    int data[ROWS / NUM_CLIENTS][COLS / NUM_CLIENTS]; /* 16x16 ũ�� ������ */
};

/* ������ �����͸� �����ϴ� �ڵ� */
void send_file_to_server(const char* file_path, int client_num) { //Ŭ���̾�Ʈ ������ ���
    int client_fd;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];
    int file_fd, bytes_read;

    /* ���� ���� */
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("���� ���� ����");
        exit(1);
    }

    /* ���� �ּ� ���� */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    if (client_num == 0 || client_num == 1 || client_num == 4 || client_num == 5)
        strncpy(server_addr.sun_path, SERVER0_PATH, sizeof(server_addr.sun_path) - 1);
    else /* 2 3 6 7 */
        strncpy(server_addr.sun_path, SERVER1_PATH, sizeof(server_addr.sun_path) - 1);

    /* ���� ���� */
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("���� ���� ����");
        close(client_fd);
        exit(1);
    }

    /* ���� ���� */
    file_fd = open(file_path, O_RDONLY);
    if (file_fd < 0) {
        perror("���� ���� ����");
        close(client_fd);
        exit(1);
    }

    /* ���� ���� ���� */
    while ((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
        if (write(client_fd, buffer, bytes_read) != bytes_read) { /* ���Ͽ��� ������ ���Ͽ� �ۼ� */
            perror("������ ���� ����");
            close(file_fd);
            close(client_fd);
            exit(1);
        }
    }

    close(file_fd);
    close(client_fd);
    printf("Ŭ���̾�Ʈ: ���� %s %s������ ���� �Ϸ�\n", file_path, server_addr.sun_path);
}

int main() {
    pid_t pids[NUM_CLIENTS];
    int msgid;
    int row, col, i, src, dest;

    /* �޽��� ť ���� */
    msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgid < 0) {
        perror("�޽��� ť ���� ����");
        exit(1);
    }

    for (i = 0; i < NUM_CLIENTS; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork ����");
            exit(1);
        }
        else if (pids[i] == 0) {
            /* Ŭ���̾�Ʈ ���μ��� */
            char path[40];
            snprintf(path, sizeof(path), "bin/client/partition/sm%d.bin", i);

            int input_fd = open(path, O_RDONLY);
            if (input_fd < 0) {
                perror("bin/client/partition/sm%d.bin : ���� ���� ����");
                exit(1);
            }

            /* 128�� �� 16�� ������ �б� */
            int matrix[ROWS][COLS / NUM_CLIENTS];
            for (row = 0; row < ROWS; row++) {
                if (read(input_fd, matrix[row], sizeof(int) * (COLS / NUM_CLIENTS))
                    != sizeof(int) * (COLS / NUM_CLIENTS)) {
                    perror("bin/client/partition/sm%d.bin : ���� �б� ����");
                    close(input_fd);
                    exit(1);
                }
            }
            close(input_fd);

            /* �޽��� �۽�: dest�� ���� �� ����� ���� */
            for (dest = 0; dest < NUM_CLIENTS; dest++) {
                struct message msg;
                msg.msg_type = dest + 1;
                msg.sender_id = i;

                int start_row = dest * (ROWS / NUM_CLIENTS); /* dest�� �ش��ϴ� 16�� ������ */
                for (row = 0; row < ROWS / NUM_CLIENTS; row++) {
                    for (col = 0; col < COLS / NUM_CLIENTS; col++) {
                        /* start_row���� 16��, �� ��� 16�� ��� ���� */
                        msg.data[row][col] = matrix[start_row + row][col];
                    }
                }

                if (msgsnd(msgid, &msg, sizeof(msg.data) + sizeof(msg.sender_id), 0) == -1) {
                    perror("�޽��� ���� ����");
                    exit(1);
                }

                printf("Client %d: �޽��� ���� �Ϸ� (to Client %d), rows [%d~%d], cols[0~15]\n",
                    i, dest, start_row, start_row + (ROWS / NUM_CLIENTS) - 1);
            }

            /* �޽��� ���� �� ���ġ */
            struct message received_msg;

            /* local_data[][]�� ���� �� ������� ���� */
            int local_data[ROWS / NUM_CLIENTS][COLS];
            memset(local_data, 0, sizeof(local_data));

            for (src = 0; src < NUM_CLIENTS; src++) {
                if (msgrcv(msgid, &received_msg, sizeof(received_msg.data) + sizeof(received_msg.sender_id),
                    i + 1, 0) == -1) {
                    perror("�޽��� ���� ����");
                    exit(1);
                }

                int sender = received_msg.sender_id;
                printf("Client %d: �޽��� ���� �Ϸ� (from Client %d)\n", i, sender);

                for (row = 0; row < ROWS / NUM_CLIENTS; row++) {
                    for (col = 0; col < COLS / NUM_CLIENTS; col++) {
                        local_data[row][col + sender * (COLS / NUM_CLIENTS)] = received_msg.data[row][col];
                    }
                }
            }

            /* ���� ��� ���� */
            snprintf(path, sizeof(path), "bin/client/collected/sm%d.bin", i);
            int output_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd < 0) {
                perror("bin/client/collected/sm%d.bin : ���� ���� ����");
                exit(1);
            }

            for (row = 0; row < ROWS / NUM_CLIENTS; row++) {
                if (write(output_fd, local_data[row], sizeof(int) * COLS) != sizeof(int) * COLS) {
                    perror("bin/client/collected/sm%d.bin : ���� ���� ����");
                    close(output_fd);
                    exit(1);
                }
            }

            close(output_fd);


            /* socket���� ������ �����͸� �����ؾ��Ѵ� */
            /* char path[40] -> char sock_path[40] ���� : client ���� �о���� ��� ������ ���ۿ� �̸� ���� */
            char sock_path[40];
            snprintf(sock_path, sizeof(sock_path), "bin/client/collected/sm%d.bin", i);
            printf("client %d send to server\n", i);
            send_file_to_server(sock_path, i);

            exit(0);

            exit(0);
        }
    }

    for (i = 0; i < NUM_CLIENTS; i++) {
        wait(NULL);
    }

    /* �޽��� ť ���� */
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("�޽��� ť ���� ����");
        exit(1);
    }

    printf("\n\n��� Ŭ���̾�Ʈ ���μ����� ����Ǿ����ϴ�.\n");
    return 0;
}
