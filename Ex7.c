#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 5
#define TOTAL_TASKS 15
#define WORKER_COUNT 5

typedef struct {
    int a;
    int b;
    char op;   /* '+', '-', '*', '/' or 0 for termination */
    int valid; /* 1 = real task, 0 = termination */
} task;

static task work_buffer[BUFFER_SIZE];
static int in = 0;
static int out = 0;

static sem_t empty;
static sem_t full;
static pthread_mutex_t mutex;      /* protects buffer indices */
static pthread_mutex_t file_mutex; /* protects FILE *fp */
static FILE *fp;

/* Pop one item from buffer into dest. Blocks until available. */
static int buffer_pop(task *dest)
{
    sem_wait(&full);
    pthread_mutex_lock(&mutex);

    *dest = work_buffer[out];
    out = (out + 1) % BUFFER_SIZE;

    pthread_mutex_unlock(&mutex);
    sem_post(&empty);

    return dest->valid;
}

/* Push one item into buffer. Blocks if full. */
static void buffer_push(const task *src)
{
    sem_wait(&empty);
    pthread_mutex_lock(&mutex);

    work_buffer[in] = *src;
    in = (in + 1) % BUFFER_SIZE;

    pthread_mutex_unlock(&mutex);
    sem_post(&full);
}

/* Worker thread argument contains worker id (1..5) */
typedef struct {
    int id;
} worker_arg_t;

void *worker_thread(void *arg)
{
    worker_arg_t wa = *(worker_arg_t *)arg;
    free(arg);
    int id = wa.id;

    /* Assign operation handled by each worker:
       1 -> '+'
       2 -> '-'
       3 -> '*'
       4 -> '/'
       5 -> will act as "dispatcher": handles any op (prints all) */
    while (1) {
        task t;
        if (!buffer_pop(&t)) break; /* termination */

        if (id == 5) {
            /* Worker 5 handles any op (dispatcher-like) */
            switch (t.op) {
                case '+': {
                    int r = t.a + t.b;
                    printf("[Thread %d] Addition: %d + %d = %d\n", id, t.a, t.b, r);
                    pthread_mutex_lock(&file_mutex);
                    if (fp) {
                        fprintf(fp, "[Thread %d] %d + %d = %d\n", id, t.a, t.b, r);
                        fflush(fp);
                    }
                    pthread_mutex_unlock(&file_mutex);
                    break;
                }
                case '-': {
                    int r = t.a - t.b;
                    printf("[Thread %d] Subtraction: %d - %d = %d\n", id, t.a, t.b, r);
                    pthread_mutex_lock(&file_mutex);
                    if (fp) {
                        fprintf(fp, "[Thread %d] %d - %d = %d\n", id, t.a, t.b, r);
                        fflush(fp);
                    }
                    pthread_mutex_unlock(&file_mutex);
                    break;
                }
                case '*': {
                    int r = t.a * t.b;
                    printf("[Thread %d] Multiplication: %d * %d = %d\n", id, t.a, t.b, r);
                    pthread_mutex_lock(&file_mutex);
                    if (fp) {
                        fprintf(fp, "[Thread %d] %d * %d = %d\n", id, t.a, t.b, r);
                        fflush(fp);
                    }
                    pthread_mutex_unlock(&file_mutex);
                    break;
                }
                case '/': {
                    if (t.b == 0) {
                        printf("[Thread %d] Division by zero skipped: %d / %d\n", id, t.a, t.b);
                        pthread_mutex_lock(&file_mutex);
                        if (fp) {
                            fprintf(fp, "[Thread %d] Division by zero skipped: %d / %d\n", id, t.a, t.b);
                            fflush(fp);
                        }
                        pthread_mutex_unlock(&file_mutex);
                    } else {
                        float r = (float)t.a / t.b;
                        printf("[Thread %d] Division: %d / %d = %.2f\n", id, t.a, t.b, r);
                        pthread_mutex_lock(&file_mutex);
                        if (fp) {
                            fprintf(fp, "[Thread %d] %d / %d = %.2f\n", id, t.a, t.b, r);
                            fflush(fp);
                        }
                        pthread_mutex_unlock(&file_mutex);
                    }
                    break;
                }
                default:
                    break;
            }
        } else {
            char myop = (id == 1) ? '+' : (id == 2) ? '-' : (id == 3) ? '*' : '/';
            if (t.op == myop) {
                if (myop == '+') {
                    int r = t.a + t.b;
                    printf("[Thread %d] Addition: %d + %d = %d\n", id, t.a, t.b, r);
                    pthread_mutex_lock(&file_mutex);
                    if (fp) {
                        fprintf(fp, "[Thread %d] %d + %d = %d\n", id, t.a, t.b, r);
                        fflush(fp);
                    }
                    pthread_mutex_unlock(&file_mutex);
                } else if (myop == '-') {
                    int r = t.a - t.b;
                    printf("[Thread %d] Subtraction: %d - %d = %d\n", id, t.a, t.b, r);
                    pthread_mutex_lock(&file_mutex);
                    if (fp) {
                        fprintf(fp, "[Thread %d] %d - %d = %d\n", id, t.a, t.b, r);
                        fflush(fp);
                    }
                    pthread_mutex_unlock(&file_mutex);
                } else if (myop == '*') {
                    int r = t.a * t.b;
                    printf("[Thread %d] Multiplication: %d * %d = %d\n", id, t.a, t.b, r);
                    pthread_mutex_lock(&file_mutex);
                    if (fp) {
                        fprintf(fp, "[Thread %d] %d * %d = %d\n", id, t.a, t.b, r);
                        fflush(fp);
                    }
                    pthread_mutex_unlock(&file_mutex);
                } else { /* '/' */
                    if (t.b == 0) {
                        printf("[Thread %d] Division by zero skipped: %d / %d\n", id, t.a, t.b);
                        pthread_mutex_lock(&file_mutex);
                        if (fp) {
                            fprintf(fp, "[Thread %d] Division by zero skipped: %d / %d\n", id, t.a, t.b);
                            fflush(fp);
                        }
                        pthread_mutex_unlock(&file_mutex);
                    } else {
                        float r = (float)t.a / t.b;
                        printf("[Thread %d] Division: %d / %d = %.2f\n", id, t.a, t.b, r);
                        pthread_mutex_lock(&file_mutex);
                        if (fp) {
                            fprintf(fp, "[Thread %d] %d / %d = %.2f\n", id, t.a, t.b, r);
                            fflush(fp);
                        }
                        pthread_mutex_unlock(&file_mutex);
                    }
                }
            } else {
                /* If op doesn't match and worker isn't dispatcher, re-enqueue task for others.
                   To avoid busy requeueing causing starvation, re-push at end of buffer. */
                buffer_push(&t);
                /* small yield to avoid tight loop */
                usleep(10000);
            }
        }
    }
    return NULL;
}

int main(void)
{
    pthread_t workers[WORKER_COUNT];
    int i;

    if (sem_init(&empty, 0, BUFFER_SIZE) != 0) {
        perror("sem_init empty");
        return 1;
    }
    if (sem_init(&full, 0, 0) != 0) {
        perror("sem_init full");
        sem_destroy(&empty);
        return 1;
    }
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        sem_destroy(&empty);
        sem_destroy(&full);
        return 1;
    }
    if (pthread_mutex_init(&file_mutex, NULL) != 0) {
        perror("pthread_mutex_init file_mutex");
        pthread_mutex_destroy(&mutex);
        sem_destroy(&empty);
        sem_destroy(&full);
        return 1;
    }

    fp = fopen("result_file.txt", "w");
    if (!fp) {
        perror("fopen");
        pthread_mutex_destroy(&file_mutex);
        pthread_mutex_destroy(&mutex);
        sem_destroy(&empty);
        sem_destroy(&full);
        return 1;
    }

    /* create workers 1..5 */
    for (i = 0; i < WORKER_COUNT; ++i) {
        worker_arg_t *wa = malloc(sizeof(worker_arg_t));
        if (!wa) { perror("malloc"); return 1; }
        wa->id = i + 1;
        if (pthread_create(&workers[i], NULL, worker_thread, wa) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    char ops[] = {'+','-','*','/'};
    srand((unsigned)time(NULL));

    /* Produce tasks */
    for (i = 0; i < TOTAL_TASKS; ++i) {
        task t;
        t.valid = 1;
        t.a = rand() % 100;
        t.b = rand() % 50; /* allow zero */
        t.op = ops[rand() % 4];

        buffer_push(&t);
        printf("Server Generated: %d %c %d\n", t.a, t.op, t.b);
        sleep(1);
    }

    /* Send termination markers: one per worker */
    for (i = 0; i < WORKER_COUNT; ++i) {
        task term = {0,0,0,0};
        buffer_push(&term);
    }

    for (i = 0; i < WORKER_COUNT; ++i) {
        pthread_join(workers[i], NULL);
    }

    /* close file under lock */
    pthread_mutex_lock(&file_mutex);
    if (fp) fclose(fp);
    fp = NULL;
    pthread_mutex_unlock(&file_mutex);

    pthread_mutex_destroy(&file_mutex);
    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);
    return 0;
}

[23bcs013@mepcolinux ex7]$cat output.txt
[23bcs013@mepcolinux ex7]$cc pro.c -lpthread -std=c99
[23bcs013@mepcolinux ex7]$./a.out
Server Generated: 34 + 30
[Thread 1] Addition: 34 + 30 = 64
Server Generated: 42 - 45
[Thread 2] Subtraction: 42 - 45 = -3
Server Generated: 95 - 7
[Thread 5] Subtraction: 95 - 7 = 88
Server Generated: 52 - 18
[Thread 2] Subtraction: 52 - 18 = 34
Server Generated: 68 / 30
[Thread 5] Division: 68 / 30 = 2.27
Server Generated: 64 - 14
[Thread 2] Subtraction: 64 - 14 = 50
Server Generated: 42 * 34
[Thread 5] Multiplication: 42 * 34 = 1428
Server Generated: 44 * 41
[Thread 3] Multiplication: 44 * 41 = 1804
Server Generated: 11 - 2
[Thread 5] Subtraction: 11 - 2 = 9
Server Generated: 54 - 41
[Thread 2] Subtraction: 54 - 41 = 13
Server Generated: 55 / 27
[Thread 5] Division: 55 / 27 = 2.04
Server Generated: 92 + 19
[Thread 1] Addition: 92 + 19 = 111
Server Generated: 97 / 16
[Thread 4] Division: 97 / 16 = 6.06
Server Generated: 14 / 20
[Thread 5] Division: 14 / 20 = 0.70
Server Generated: 27 - 41
[Thread 2] Subtraction: 27 - 41 = -14
[23bcs013@mepcolinux ex7]$ls
a.out  pro1.c  pro.c  result_file.txt
[23bcs013@mepcolinux ex7]$cat result_file.txt
[Thread 1] 34 + 30 = 64
[Thread 2] 42 - 45 = -3
[Thread 5] 95 - 7 = 88
[Thread 2] 52 - 18 = 34
[Thread 5] 68 / 30 = 2.27
[Thread 2] 64 - 14 = 50
[Thread 5] 42 * 34 = 1428
[Thread 3] 44 * 41 = 1804
[Thread 5] 11 - 2 = 9
[Thread 2] 54 - 41 = 13
[Thread 5] 55 / 27 = 2.04
[Thread 1] 92 + 19 = 111
[Thread 4] 97 / 16 = 6.06
[Thread 5] 14 / 20 = 0.70
[Thread 2] 27 - 41 = -14
