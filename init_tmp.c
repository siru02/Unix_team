#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "config.h"

int main() {
    FILE* file;
    int matrix[ROWS][COLS];
    int cols_per_client = COLS / OFFSET;
    int i, j, client;

    /* bin/ 필수 디렉토리 생성 */
    if (access("bin", F_OK) == -1) {
        if (mkdir("bin", 0777) == -1) {
            perror("bin mkdir failed");
            exit(1);
        }
        printf("bin 디렉토리가 생성되었습니다.\n");
    }

    if (access("bin/client", F_OK) == -1) {
        if (mkdir("bin/client", 0777) == -1) {
            perror("bin/client mkdir failed");
            exit(1);
        }
        printf("bin/client 디렉토리가 생성되었습니다.\n");
    }

    if (access("bin/server", F_OK) == -1) {
        if (mkdir("bin/server", 0777) == -1) {
            perror("bin/server mkdir failed");
            exit(1);
        }
        printf("bin/server 디렉토리가 생성되었습니다.\n");
    }

    if (access("bin/server/server0", F_OK) == -1) {
        if (mkdir("bin/server/server0", 0777) == -1) {
            perror("bin/server/server0 mkdir failed");
            exit(1);
        }
        printf("bin/server/server0 디렉토리가 생성되었습니다.\n");
    }

    if (access("bin/server/server1", F_OK) == -1) {
        if (mkdir("bin/server/server1", 0777) == -1) {
            perror("bin/server/server1 mkdir failed");
            exit(1);
        }
        printf("bin/server/server1 디렉토리가 생성되었습니다.\n");
    }

    if (access("bin/client/partition", F_OK) == -1) {
        if (mkdir("bin/client/partition", 0777) == -1) {
            perror("bin/client/partition mkdir failed");
            exit(1);
        }
        printf("bin/client/partition 디렉토리가 생성되었습니다.\n");
    }

    if (access("bin/client/collected", F_OK) == -1) {
        if (mkdir("bin/client/collected", 0777) == -1) {
            perror("bin/client/collected mkdir failed");
            exit(1);
        }
        printf("bin/client/collected 디렉토리가 생성되었습니다.\n");
    }

    /* data.bin 생성 */
    file = fopen("bin/data.bin", "w");
    if (file == NULL) {
        printf("파일을 열 수 없습니다.\n");
        return 1;
    }

    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            int value = i * ROWS + j;
            fwrite(&value, sizeof(int), 1, file);
        }
    }

    fclose(file);
    printf("데이터 파일이 생성되었습니다: bin/data.bin\n");

    /* data.bin -> 8개 분산된 sm_i.bin 생성 */
    int input_fd = open("bin/data.bin", O_RDONLY);
    if (input_fd < 0) {
        perror("bin/data.bin 파일을 열 수 없습니다");
        return 1;
    }

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

    /* 8개의 클라이언트 파일 생성, 분배 후 저장 */
    for (client = 0; client < NUM_CLIENTS; client++) {
        char filename[20];
        sprintf(filename, "bin/client/partition/sm%d.bin", client);

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

    return 0;
}
