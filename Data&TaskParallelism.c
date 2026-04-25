#define _POSIX_C_SOURCE 199309L
#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <time.h>

#define N 500  // maximum matrix size

// Function to generate random matrix
void generateMatrix(int n, int **A) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            A[i][j] = rand() % 10;
}

// Function to print matrix
void printMatrix(int n, int **A) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            printf("%d ", A[i][j]);
        printf("\n");
    }
}
void printSharedMatrix(int n, int (*A)[n]) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            printf("%d ", A[i][j]);
        printf("\n");
    }
}

int main() {
    srand(time(NULL));
    int n;
    printf("Enter matrix limit:");
    scanf("%d",&n);
    //int n = N;
    int **matA = malloc(n * sizeof(int *));
    int **matB = malloc(n * sizeof(int *));
    int **matC_serial=malloc(n*sizeof(int *));

    for (int i = 0; i < n; i++) {
        matA[i] = malloc(n * sizeof(int));
        matB[i] = malloc(n * sizeof(int));
        matC_serial[i] = malloc(n * sizeof(int));
     }

    printf("Matrix size: %dx%d\n\n", n, n);

    generateMatrix(n, matA);
    generateMatrix(n, matB);

    if(n<=5){
       printf("Matrix A:\n");
       printMatrix(n, matA);
       printf("\nMatrix B:\n");
       printMatrix(n, matB);
    }

    struct timespec start, end;

    // ================= SERIAL EXECUTION =================
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matC_serial[i][j] = 0;
            for (int k = 0; k < n; k++) {
                if(n<=5){
            //    printf("Serial: C[%d][%d] += A[%d][%d] * B[%d][%d]\n",
              //          i, j, i, k, k, j);
                }
                matC_serial[i][j] += matA[i][k] * matB[k][j];
            }
        }
    }

  clock_gettime(CLOCK_MONOTONIC, &end);
        double serial_time =
                   (end.tv_sec - start.tv_sec)*1000 +
                           (end.tv_nsec - start.tv_nsec) / 1000000;

    if(n<=5){
       printf("\nSerial Result Matrix:\n");
       printMatrix(n, matC_serial);
    }

    // ================= PARALLEL EXECUTION =================
    int shmid = shmget(IPC_PRIVATE, sizeof(int) * n * n, IPC_CREAT | 0666);
    int (*matC_parallel)[n] = shmat(shmid, NULL, 0);

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            for (int j = 0; j < n; j++) {
                matC_parallel[i][j] = 0;
                for (int k = 0; k < n; k++) {
                    if(n<=5){
        //                printf("Child %d: C[%d][%d] += A[%d][%d] * B[%d][%d]\n",
        //                    getpid(), i, j, i, k, k, j);
                       }
                        matC_parallel[i][j] += matA[i][k] * matB[k][j];
                }
            }
            exit(0);
        }
    }

    for (int i = 0; i < n; i++)
        wait(NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double parallel_time =
        (end.tv_sec - start.tv_sec)*1000 +
        (end.tv_nsec - start.tv_nsec) / 1000000;

    if(n<=5){
       printf("\nParallel Result Matrix:\n");
       printSharedMatrix(n, matC_parallel);
    }
    printf("\nExecution Time:\n");
    printf("Serial Time   : %lf seconds\n", serial_time);
    printf("Parallel Time : %lf seconds\n", parallel_time);

    shmdt(matC_parallel);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
[23bcs013@mepcolinux HPC]$cc pro1.c -std=c99 -o exe1.exe
[23bcs013@mepcolinux HPC]$./exe1.exe
Enter matrix limit:5
Matrix size: 5x5

Matrix A:
1 1 1 4 1
5 7 1 8 6
1 4 1 3 6
9 9 9 2 7
6 6 4 8 7

Matrix B:
0 7 7 9 7
8 0 0 0 6
1 5 3 5 3
9 8 9 2 2
5 1 3 7 3

Serial Result Matrix:
50 45 49 29 27
159 110 128 108 114
90 42 55 62 58
134 131 129 179 169
159 133 147 139 127

Parallel Result Matrix:
50 45 49 29 27
159 110 128 108 114
90 42 55 62 58
134 131 129 179 169
159 133 147 139 127

Execution Time:
Serial Time   : 0.000000 seconds
Parallel Time : 12.000000 seconds
[23bcs013@mepcolinux HPC]$vi pro1.c
[23bcs013@mepcolinux HPC]$./exe1.exe
Enter matrix limit:500
Matrix size: 500x500


Execution Time:
Serial Time   : 1738.000000 seconds
Parallel Time : 29.000000 seconds
[23bcs013@mepcolinux HPC]$./exe1.exe
Enter matrix limit:50
Matrix size: 50x50


Execution Time:
Serial Time   : 1.000000 seconds
Parallel Time : 29.000000 seconds
[23bcs013@mepcolinux HPC]$./exe1.exe
Enter matrix limit:1000
Matrix size: 1000x1000


Execution Time:
Serial Time   : 24297.000000 seconds
Parallel Time : 61.000000 seconds

[23bcs013@mepcolinux HPC]$cat pro2.c
#define _POSIX_C_SOURCE 199309L
#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define N 5

// Function to generate random matrix
void generateMatrix(int n, int **A) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            A[i][j] = rand() % 10;
}

// Print int ** matrix
void printMatrix(int n, int **A) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            printf("%d ", A[i][j]);
        printf("\n");
    }
}

int main() {
    srand(time(NULL));
    int n;
    printf("Enter matrix limit:");
    scanf("%d",&n);

    // Allocate matrices dynamically
    int **matA = malloc(n * sizeof(int *));
    int **matB = malloc(n * sizeof(int *));
    int **mat_add = malloc(n * sizeof(int *));
    int **mat_sub = malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++) {
        matA[i] = malloc(n * sizeof(int));
        matB[i] = malloc(n * sizeof(int));
        mat_add[i] = malloc(n * sizeof(int));
        mat_sub[i] = malloc(n * sizeof(int));
    }

    printf("Matrix size: %dx%d\n\n", n, n);

    generateMatrix(n, matA);
    generateMatrix(n, matB);

    if (n <= 5) {
        printf("Matrix A:\n"); printMatrix(n, matA);
        printf("\nMatrix B:\n"); printMatrix(n, matB);
    }

    struct timespec start, end;

    // ================= SERIAL EXECUTION =================
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++) {
            mat_add[i][j] = matA[i][j] + matB[i][j];

        //    if (n <= 5)
 //               printf("Serial: add[%d][%d]=%d\n",
   //                    i,j,mat_add[i][j]);
        }
    }
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++) {
            mat_sub[i][j] = matA[i][j] - matB[i][j];

           // if (n <= 5)
       //         printf("Serial: sub[%d][%d]=%d\n",i,j,mat_sub[i][j]);
        }
    }


    clock_gettime(CLOCK_MONOTONIC, &end);
    double serial_time =
        (end.tv_sec - start.tv_sec)*1000 +
        (end.tv_nsec - start.tv_nsec)/1000000;

    if (n <= 5) {
        printf("\nSerial Addition Result:\n"); printMatrix(n, mat_add);
        printf("\nSerial Subtraction Result:\n"); printMatrix(n, mat_sub);
    }

    // ================= PARALLEL EXECUTION =================

    double pll_time=0,parallel_time=0,pal_time=0;
    pid_t pid = fork();
    pll_time=0;
    if (pid == 0) {
        // -------- CHILD PROCESS: subtraction --------
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int i = 0; i < n; i++){
            for (int j = 0; j < n; j++){
                mat_sub[i][j] = matA[i][j] - matB[i][j];
            }
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        pll_time =(end.tv_sec - start.tv_sec)*1000 +(end.tv_nsec - start.tv_nsec)/1000000;
        if(pll_time>parallel_time){
                parallel_time=pll_time;
        }
        if (n <= 5) {
            printf("\nChild Process: Subtraction Result:\n");
            printMatrix(n, mat_sub);
        }
        exit(0);
    } else {
        // -------- PARENT PROCESS: addition --------
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int i = 0; i < n; i++){
            for (int j = 0; j < n; j++){
                mat_add[i][j] = matA[i][j] + matB[i][j];
            }
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        pal_time =(end.tv_sec - start.tv_sec)*1000 +(end.tv_nsec - start.tv_nsec)/1000000;
        if(pal_time>parallel_time){
                parallel_time=pal_time;
        }
        if (n <= 5) {
            printf("\nParent Process: Addition Result:\n");
            printMatrix(n, mat_add);
        }
        wait(NULL);
    }


    printf("\nExecution Time:\n");
    printf("Serial Time   : %lf seconds\n", serial_time);
    printf("Parallel Time : %lf seconds\n", parallel_time);

    // Free memory
  for (int i = 0; i < n; i++) {
        free(matA[i]);
        free(matB[i]);
        free(mat_add[i]);
        free(mat_sub[i]);
    }
    free(matA);
    free(matB);
    free(mat_add);
    free(mat_sub);

    return 0;
}
[23bcs013@mepcolinux HPC]$./exe2.exe
Enter matrix limit:5
Matrix size: 5x5

Matrix A:
4 8 4 8 1
6 3 6 0 9
4 5 2 0 4
8 5 8 8 7
8 7 6 7 7

Matrix B:
2 9 5 5 6
6 9 5 3 9
6 9 3 4 1
2 0 8 6 0
3 6 6 1 6

Serial Addition Result:
6 17 9 13 7
12 12 11 3 18
10 14 5 4 5
10 5 16 14 7
11 13 12 8 13

Serial Subtraction Result:
2 -1 -1 3 -5
0 -6 1 -3 0
-2 -4 -1 -4 3
6 5 0 2 7
5 1 0 6 1

Parent Process: Addition Result:
6 17 9 13 7
12 12 11 3 18
10 14 5 4 5
10 5 16 14 7
11 13 12 8 13

Child Process: Subtraction Result:
2 -1 -1 3 -5
0 -6 1 -3 0
-2 -4 -1 -4 3
6 5 0 2 7
5 1 0 6 1

Execution Time:
Serial Time   : 0.000000 seconds
Parallel Time : 0.000000 seconds
[23bcs013@mepcolinux HPC]$./exe2.exe
Enter matrix limit:50
Matrix size: 50x50
Execution Time:
Serial Time   : 0.000000 seconds
Parallel Time : 0.000000 seconds
[23bcs013@mepcolinux HPC]$./exe2.exe
Enter matrix limit:500
Matrix size: 500x500
Execution Time:
Serial Time   : 4.000000 seconds
Parallel Time : 4.000000 seconds
[23bcs013@mepcolinux HPC]$./exe2.exe
Enter matrix limit:100
Matrix size: 100x100
Execution Time:
Serial Time   : 0.000000 seconds
Parallel Time : 0.000000 seconds
[23bcs013@mepcolinux HPC]$./exe2.exe
Enter matrix limit:10000
Matrix size: 10000x10000
Execution Time:
Serial Time   : 2962.000000 seconds
Parallel Time : 2083.000000 seconds
[23bcs013@mepcolinux HPC]$./exe2.exe
Enter matrix limit:500
Matrix size: 500x500
Execution Time:
Serial Time   : 5.000000 seconds
Parallel Time : 6.000000 seconds
[23bcs013@mepcolinux HPC]$./exe2.exe
Enter matrix limit:5000
Matrix size: 5000x5000
Execution Time:
Serial Time   : 826.000000 seconds
Parallel Time : 624.000000 seconds
[23bcs013@mepcolinux HPC]$./exe2.exe
Enter matrix limit:1000
Matrix size: 1000x1000
Execution Time:
Serial Time   : 45.000000 seconds
Parallel Time : 28.000000 seconds
