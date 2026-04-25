III-A1@cil26:~$ cat ex5.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
int **A, **B;
int **sum;
int **product;
int SIZE;
double get_elapsed_ms(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;
}
void* add_matrices(void* arg)
{
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            sum[i][j] = A[i][j] + B[i][j];
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double *elapsed = malloc(sizeof(double));
    *elapsed = get_elapsed_ms(start, end);
    printf("\n[Thread 1] Matrix Addition Complete.\n");
    pthread_exit(elapsed);
}
void* multiply_matrices(void* arg)
{
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            product[i][j] = 0;
            for (int k = 0; k < SIZE; k++)
            {
                product[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double *elapsed = malloc(sizeof(double));
    *elapsed = get_elapsed_ms(start, end);
    printf("[Thread 2] Matrix Multiplication Complete.\n");
    pthread_exit(elapsed);
}
int main()
{
    pthread_t thread1, thread2;
    double *time1, *time2;
    printf("Enter matrix size (N for NxN): ");
    scanf("%d", &SIZE);
    srand(time(NULL));
    A = malloc(SIZE * sizeof(int *));
    B = malloc(SIZE * sizeof(int *));
    sum = malloc(SIZE * sizeof(int *));
    product = malloc(SIZE * sizeof(int *));
    for (int i = 0; i < SIZE; i++)
    {
        A[i] = malloc(SIZE * sizeof(int));
        B[i] = malloc(SIZE * sizeof(int));
        sum[i] = malloc(SIZE * sizeof(int));
        product[i] = malloc(SIZE * sizeof(int));
        for (int j = 0; j < SIZE; j++)
        {
            A[i][j] = rand() % 10;
            B[i][j] = rand() % 10;
        }
    }
    pthread_create(&thread1, NULL, add_matrices, NULL);
    pthread_create(&thread2, NULL, multiply_matrices, NULL);
    pthread_join(thread1, (void**)&time1);
    pthread_join(thread2, (void**)&time2);
    printf("\n--- Execution Times ---");
    printf("\nAddition Thread: %.4f ms", *time1);
    printf("\nMultiplication Thread: %.4f ms\n", *time2);
    if (SIZE < 6)
    {
        printf("\n--- Matrix A ---\n");
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++) printf("%d ", A[i][j]);
            printf("\n");
        }
        printf("\n--- Matrix B ---\n");
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++) printf("%d ", B[i][j]);
            printf("\n");
        }
        printf("\n--- Result of Addition ---\n");
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++) printf("%d ", sum[i][j]);
            printf("\n");
        }
        printf("\n--- Result of Multiplication ---\n");
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++) printf("%d ", product[i][j]);
            printf("\n");
        }
    }
    else
    {
        printf("\nMatrix size is %dx%d. Skipping matrix display (Size >= 6x6).\n", SIZE, SIZE);
    }
    for (int i = 0; i < SIZE; i++)
    {
        free(A[i]); free(B[i]); free(sum[i]); free(product[i]);
    }
    free(A); free(B); free(sum); free(product);
    free(time1); free(time2);
    return 0;
}

III-A1@cil26:~$
III-A1@cil26:~$ cc ex5.c -lpthread
III-A1@cil26:~$ ./a.out
Enter matrix size (N for NxN): 5
[Thread 2] Matrix Multiplication Complete.

[Thread 1] Matrix Addition Complete.

--- Execution Times ---
Addition Thread: 0.0015 ms
Multiplication Thread: 0.0035 ms

--- Matrix A ---
2 8 7 7 8
2 6 1 4 5
9 9 3 9 0
8 3 9 4 2
5 8 0 7 9

--- Matrix B ---
1 5 5 9 4
2 4 5 8 3
5 5 8 6 2
4 8 2 9 4
6 2 1 7 5

--- Result of Addition ---
3 13 12 16 12
4 10 6 12 8
14 14 11 15 2
12 11 11 13 6
11 10 1 14 14

--- Result of Multiplication ---
129 149 128 243 114
65 81 61 143 69
78 168 132 252 105
87 133 137 200 85
103 131 88 235 117


