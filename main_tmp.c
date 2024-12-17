#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <limits.h>
#include <time.h>
#include "config.h"

#define SERVER0_PATH "./socket/server0_socket"
#define SERVER1_PATH "./socket/server1_socket"
#define BUFFER_SIZE 4096
#define NUM_CLIENT 4

/* ���� ���� �Լ� */
void merge_files(int server_id) {
    int client_fds[NUM_CLIENT];  /* Ŭ���̾�Ʈ ���� ��ũ���� */
    int current_values[NUM_CLIENT];  /* �� ������ ���� �� */
    int valid_fd[NUM_CLIENT] = { 1, 1, 1, 1 };  /* ������ ���� ���� �����Ͱ� �ִ��� */
    int active_files = NUM_CLIENT;  /* ���� ó���ؾ� �� ���� �� */
    int i;

 /* ���� �ð� ������ ���� ���� */
    struct timespec merge_start, merge_end;
    double merge_time;

    /* ���� ���� �ð� ���� */
    clock_gettime(CLOCK_MONOTONIC, &merge_start);

    /* ��� ���� ���� */
    int output_fd;
    if (server_id == 0) {
        output_fd = open("bin/server/server0/server0.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }
    else {
        output_fd = open("bin/server/server1/server1.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }

    if (output_fd < 0) {
        perror("��� ���� ���� ����");
        return;
    }

    /* �Է� ���ϵ� ���� */
    for (i = 0; i < NUM_CLIENT; i++) {
        char path[256];
        if (server_id == 0) {
            sprintf(path, "bin/server/server0/client%d.bin", i);
        }
        else {
            sprintf(path, "bin/server/server1/client%d.bin", i);
        }

        client_fds[i] = open(path, O_RDONLY);
        if (client_fds[i] < 0) {
            perror("�Է� ���� ���� ����");
            valid_fd[i] = 0;
            active_files--;
            continue;
        }

        /* �� ������ ù �� �б� */
        if (read(client_fds[i], &current_values[i], sizeof(int)) != sizeof(int)) {
            close(client_fds[i]);
            valid_fd[i] = 0;
            active_files--;
        }
    }

    /* ��� ������ �����͸� ���ϸ� ���� */
    while (active_files > 0) {
        /* ���� ���� ���� �� ã�� */
        int min_value = INT_MAX;
        int min_idx = -1;

        for (i = 0; i < NUM_CLIENT; i++) {
            if (valid_fd[i] && current_values[i] < min_value) {
                min_value = current_values[i];
                min_idx = i;
            }
        }

        if (min_idx != -1) {
            /* ���� ���� �� ���� */
            write(output_fd, &current_values[min_idx], sizeof(int));

            /* ���õ� ���Ͽ��� ���� �� �б� */
            if (read(client_fds[min_idx], &current_values[min_idx], sizeof(int)) != sizeof(int)) {
                close(client_fds[min_idx]);
                valid_fd[min_idx] = 0;
                active_files--;
            }
        }
    }

    /* ���� ��ũ���� ���� */
    close(output_fd);
    for (i = 0; i < NUM_CLIENT; i++) {
        if (valid_fd[i]) {
            close(client_fds[i]);
        }
    }
/* ���� ���� �ð� ���� �� �ҿ� �ð� ��� */
    clock_gettime(CLOCK_MONOTONIC, &merge_end);
    merge_time = (merge_end.tv_sec - merge_start.tv_sec) + (merge_end.tv_nsec - merge_start.tv_nsec) / 1e9;
    printf("���� %d: ���� ���� �۾� �ҿ� �ð�: %.6f ��\n", server_id, merge_time);
}

/* ����i ���μ��� ���� �Լ� */
void server_process(const char* socket_path, int server_id) {
    int server_fd, client_fd;
    struct sockaddr_un server_addr;
    int connected_clients = 0;
    fd_set active_fd_set, read_fd_set;
    struct timespec start, end;
    double elapsed_time;
    int is_first_client = 1;

    /* ���� ���� */
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("���� ���� ����");
        exit(1);
    }

    /* ���� �ּ� ���� */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);

    /* ���� ���� ���� ���� */
    unlink(socket_path);

    /* ���� ���ε� */
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("���� ���ε� ����");
        close(server_fd);
        exit(1);
    }

    /* ���� ��� */
    if (listen(server_fd, NUM_CLIENT) < 0) {
        perror("���� ��� ����");
        close(server_fd);
        exit(1);
    }

    printf("���� %d: %s ���� ��� ��...\n", server_id, socket_path);

    /* fd_set �ʱ�ȭ */
    FD_ZERO(&active_fd_set);
    FD_SET(server_fd, &active_fd_set);

    while (connected_clients < NUM_CLIENT) {
        read_fd_set = active_fd_set;

        if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
            perror("select ����");
            exit(1);
        }

        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("Ŭ���̾�Ʈ ���� ����");
            continue;
        }

        /* ù Ŭ���̾�Ʈ ���� �� �ð� ���� ���� */
        if (is_first_client) {
            clock_gettime(CLOCK_MONOTONIC, &start);
            is_first_client = 0;
        }

        printf("���� %d: Ŭ���̾�Ʈ %d ���� ����\n", server_id, connected_clients);

        /* Ŭ���̾�Ʈ ���� ���� */
        int output_fd;
        if (server_id == 0) {
            if (connected_clients == 0) output_fd = open("bin/server/server0/client0.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            else if (connected_clients == 1) output_fd = open("bin/server/server0/client1.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            else if (connected_clients == 2) output_fd = open("bin/server/server0/client2.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            else output_fd = open("bin/server/server0/client3.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }
        else {
            if (connected_clients == 0) output_fd = open("bin/server/server1/client0.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            else if (connected_clients == 1) output_fd = open("bin/server/server1/client1.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            else if (connected_clients == 2) output_fd = open("bin/server/server1/client2.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            else output_fd = open("bin/server/server1/client3.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }

        if (output_fd < 0) {
            perror("���� ���� ����");
            close(client_fd);
            continue;
        }

        /* ������ ���� �� ���� ���� */
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;
        while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
            if (write(output_fd, buffer, bytes_read) != bytes_read) {
                perror("���� ���� ����");
                break;
            }
        }

        close(output_fd);
        close(client_fd);
        printf("���� %d: Ŭ���̾�Ʈ %d ������ ���� �Ϸ�\n", server_id, connected_clients);
        connected_clients++;
    }

    /* ��� �����͸� ���� �� �ð� ���� ���� */
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("���� %d: ��� Ŭ���̾�Ʈ ������ ���� �Ϸ�. �ҿ� �ð�: %.6f ��\n", server_id, elapsed_time);


    /* ��� Ŭ���̾�Ʈ ������ ���� */
    merge_files(server_id);

    close(server_fd);
    unlink(socket_path);
    printf("���� %d: ��� ������ ���� �Ϸ�\n", server_id);
    exit(0);
}

int main() {
    pid_t pid0, pid1;
    int status;

    /* ����0 ���� */
    if ((pid0 = fork()) == 0) {
        server_process(SERVER0_PATH, 0);
    }

    /* ����1 ���� */
    if ((pid1 = fork()) == 0) {
        server_process(SERVER1_PATH, 1);
    }

    /* �θ� ���μ����� ����0�� ����1�� ���� ��� */
    waitpid(pid0, &status, 0);
    if (WIFEXITED(status)) {
        printf("���� 0�� ���� ����� (exit status: %d)\n", WEXITSTATUS(status));
    }

    waitpid(pid1, &status, 0);
    if (WIFEXITED(status)) {
        printf("���� 1�� ���� ����� (exit status: %d)\n", WEXITSTATUS(status));
    }

    printf("��� ���� ���μ��� ����\n");
    return 0;
}
