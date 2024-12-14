#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "config.h"
//#include <cuda_runtime_api.h>

//#define ROWS 128
//#define COLS 128

int main() {
    FILE *file;
    file = fopen("bin/data.bin", "w");

    if(access("bin/client", F_OK) == -1){
		if(mkdir("bin/client", 0777) == -1){
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

    if(access("bin/client/partition", F_OK) == -1){
		if(mkdir("bin/client/partition", 0777) == -1){
	    	perror("bin/client/partition mkdir failed");
	    	exit(1);
		}
		printf("bin/client/partition 디렉토리가 생성되었습니다.\n");
    }
  
    if(access("bin/client/collected", F_OK) == -1){
        if(mkdir("bin/client/collected", 0777) == -1){
            perror("bin/client/collected mkdir failed");
            exit(1);
        }
        printf("bin/client/collected 디렉토리가 생성되었습니다.\n");
    }
 

    if (file == NULL) {
        printf("파일을 열 수 없습니다.\n");
        return 1;
    }

	int i, j;
	for (i = 0; i < ROWS; i++) {
        	for (j = 0; j < COLS; j++) {
			//fprintf(file, "%d ", i * ROWS + j);
        		int value = i * ROWS + j;	
			fwrite(&value, sizeof(int), 1, file);
		}
        	//fprintf(file, "\n"); // 행마다 줄바꿈
    	}

	fclose(file);
    	printf("데이터 파일이 생성되었습니다: bin/data.bin\n");
    	return 0;
}
