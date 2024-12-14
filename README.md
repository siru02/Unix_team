# Unix_Term-Project_CUDA
### parallel operation project with CUDA
```
.
├── README.md
├── bin                      // binary files
│   ├── client
│   │   ├── collected        // by client-client comm. 
│   │   │   ├── sm0.bin
│   │   │   ├── sm1.bin
│   │   │   ├── sm2.bin
│   │   │   ├── sm3.bin
│   │   │   ├── sm4.bin
│   │   │   ├── sm5.bin
│   │   │   ├── sm6.bin
│   │   │   └── sm7.bin
│   │   └── partition        // 8*8 partitioning | 4*4 partitioning
│   │       ├── sm0.bin
│   │       ├── sm1.bin
│   │       ├── sm2.bin
│   │       ├── sm3.bin
│   │       ├── sm4.bin
│   │       ├── sm5.bin
│   │       ├── sm6.bin
│   │       └── sm7.bin
│   ├── data.bin              // 128*128 matrix
│   └── server                //
│
├── client.c      // client-client comm & client-server comm
├── config.h      // header file 
├── init.c        // setup & directory setting
└── main.c        // displit data set

```

makeDataSet.c -> main.c
----
# makeDataSet.c
>input, output directory 생성 : **sm_0.txt**(input directory) => **sm_0.out.txt**(output directory)
>
>
>
>config.h에 세팅 된 ROWS, COLS 값에 의해 MATRIX 생성

# main.c
>input_data.txt 읽어와서 matrix 세로로 8등분 후 sm_0.txt ~ sm_7.txt 분배, 저장

# client.c
>Loop -> fork() 8회 호출
>
>각 자식 프로세스는 **8개의 client(SM)** 역할
> 
> 
>fork된 자식 프로세스는 실제로는 CPU스케줄러에 의해 순서가 정해진 채 실행되지만
>
>**프로젝트 상에는 CUDA를 이용해 병렬 처리 한다고 가정**
>
>연산 과정은 그냥 입력 data set의 정수에 1씩 더한다고 가정
> 
>각 SM은 자신에게 할당된 파일 하나를 input/에서 읽어와서 1씩 더하고 다시 output/sm_i.out.txt 에 저장
>


```
pid_t pids[NUM_CLIENTS];
int i;
for(i=0; i<NUM_CLIENTS; i++){
        pids[i] = fork();
        if(pids[i] < 0){
                perror("fork failed");
                exit(1);
        }else if(pids[i] == 0){
                // client i (sm i)
                // 각 자식 프로세스들은 스케쥴러에 의해 실행되지만
                // 동시에 병렬 처리된다고 가정
                // sm_i.txt 파일 읽어옴
                // output_file = "output/sm_i.out.txt" 에 1씩 더해서 저장
        }
}

```

# server.c
서버0은 sm0, sm1, sm4, sm5
서버1은 sm2, sm3, sm6, sm7

1. 서버와 클라이언트가 소켓으로 연결하여 파일을 받기 -> 시간측정1
2. 서버가 클라이언트들로부터 받은 데이터를 파일에 쓰기 -> 시간측정2







# Idea









