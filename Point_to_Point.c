#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <mpi.h>

const int MAX_STRING = 100;

/* Palindrome check function */
int isPalindrome(char str[]) {
    int i = 0, j = strlen(str) - 1;
    while (i < j) {
        if (str[i] != str[j])
            return 0;
        i++;
        j--;
    }
    return 1;
}

int main(void) {
    char message[MAX_STRING];
    int comm_sz;          /* Number of processes */
    int my_rank;          /* Process rank */
    MPI_Status status;

    /* List of strings */
    char *string_list[] = {
        "radar",
        "hello",
        "level",
        "world",
        "madam",
        "computer",
    };
    int list_size = sizeof(string_list) / sizeof(string_list[0]);

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    double start_time = MPI_Wtime(); // Start timing

    /* Non-master processes */
    if (my_rank != 0) {
        /* Pick string from list */
        strcpy(message, string_list[my_rank % list_size]);

        if (my_rank % 2 == 1) {
            /* Odd rank → Type 1 (UPPERCASE only) */
            MPI_Send(message, strlen(message) + 1,
                     MPI_CHAR, 0, 1, MPI_COMM_WORLD);
        } else {
            /* Even rank → Type 2 (Palindrome only) */
            MPI_Send(message, strlen(message) + 1,
                     MPI_CHAR, 0, 2, MPI_COMM_WORLD);
        }

        // End timing for this worker process
//        double execution_time = MPI_Wtime() - start_time; // End timing
//        printf("Execution time for process %d: %f milli seconds\n", my_rank, execution_time*1000);
    }
    /* Master process */
    else {
        char recv_msg[MAX_STRING];

        for (int q = 1; q < comm_sz; q++) {
            MPI_Recv(recv_msg, MAX_STRING, MPI_CHAR,
                     MPI_ANY_SOURCE, MPI_ANY_TAG,
                     MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == 1) {
                /* Convert ONLY Type 1 to uppercase */
                for (int i = 0; recv_msg[i]; i++)
                    recv_msg[i] = toupper(recv_msg[i]);

                printf("From process %d | Type 1 | Uppercase: %s\t",
                       status.MPI_SOURCE, recv_msg);
            } else if (status.MPI_TAG == 2) {
                /* No conversion here */
                if (isPalindrome(recv_msg))
                    printf("From process %d | Type 2 | Palindrome: %s\t",
                           status.MPI_SOURCE, recv_msg);
                else
                    printf("From process %d | Type 2 | Not Palindrome: %s\t",
                           status.MPI_SOURCE, recv_msg);
            }
           double execution_time = MPI_Wtime() - start_time; // End timing

           printf("|    Execution time for process %d: %f milli seconds\n",status.MPI_SOURCE, execution_time*1000);
        }

        // End timing for the master process
        double total_execution_time = MPI_Wtime() - start_time; // End timing
        printf("Total execution time for master process: %f milli seconds\n", total_execution_time*1000);
    }

    MPI_Finalize();
    return 0;
}

[23bcs013@mepcolinux ex2]$cat op.txt
III-A1@cil26:~$ mpicc exp2.c
III-A1@cil26:~$ mpiexec -n 6 ./a.out
From process 5 | Type 1 | Uppercase: HELLO      |    Execution time for process 5: 0.028353 milli seconds
From process 1 | Type 1 | Uppercase: MADAM      |    Execution time for process 1: 0.037453 milli seconds
From process 3 | Type 1 | Uppercase: IMAGE      |    Execution time for process 3: 0.039340 milli seconds
From process 4 | Type 2 | Not Palindrome: letter        |    Execution time for process 4: 0.041014 milli seconds
From process 2 | Type 2 | Not Palindrome: work  |    Execution time for process 2: 0.048063 milli seconds
Total execution time for master process: 0.049629 milli seconds

