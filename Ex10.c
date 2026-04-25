#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define MAX_QUEUE 100
#define MAX_LINE 256

char *queue[MAX_QUEUE];
int front = 0, rear = 0, count = 0;
omp_lock_t lock;
int done = 0;

void enqueue(char *line) {
    omp_set_lock(&lock);
    if (count < MAX_QUEUE) {
        queue[rear] = (char *)malloc(strlen(line) + 1);
        strcpy(queue[rear], line);
        rear = (rear + 1) % MAX_QUEUE;
        count++;
    }
    omp_unset_lock(&lock);
}

char* dequeue() {
    char *line = NULL;
    omp_set_lock(&lock);
    if (count > 0) {
        line = queue[front];
        front = (front + 1) % MAX_QUEUE;
        count--;
    }
    omp_unset_lock(&lock);
    return line;
}

int main() {
    int num_producers = 2, num_consumers = 2;
    omp_init_lock(&lock);

    #pragma omp parallel num_threads(num_producers + num_consumers)
    {
        int id = omp_get_thread_num();

        if (id < num_producers) {
            char filename[20];
            sprintf(filename, "file%d.txt", id + 1);
            FILE *fp = fopen(filename, "r");
            if (fp) {
                char line[MAX_LINE];
                while (fgets(line, MAX_LINE, fp)) {
                    enqueue(line);
                    #pragma omp critical
                    printf("Producer %d added: %s", id, line);
                }
                fclose(fp);
            }
            #pragma omp atomic
            done++;
        }
        else {
            while (1) {
                char *line = dequeue();
                if (line != NULL) {
                    #pragma omp critical
                    {
                        printf("Consumer %d tokens:\n", id);
                        char *saveptr;
                        // Use strtok_r for thread-safe tokenizing
                        char *token = strtok_r(line, " \t\n\r", &saveptr);
                        while (token != NULL) {
                            printf("%s\n", token); // Print each token on a new line
                            token = strtok_r(NULL, " \t\n\r", &saveptr);
                        }
                    }
                    free(line);
                } else {
                    int check_done;
                    #pragma omp atomic read
                    check_done = done;
                    if (check_done == num_producers && count == 0) break;
                }
            }
        }
    }
    omp_destroy_lock(&lock);
    return 0;
}


[23bcs013@mepcolinux ex10]$cc ex10.c -fopenmp
[23bcs013@mepcolinux ex10]$./a.out
Producer 0 added: good afternoon
Consumer 2 tokens:
good
afternoon
Producer 0 added: I'm a student
Consumer 3 tokens:
this
is
HPC
lab
Consumer 2 tokens:
I'm
a
student
Producer 1 added: this is HPC lab
Producer 1 added: How you compete
Consumer 3 tokens:
How
you
compete
