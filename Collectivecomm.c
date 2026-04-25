III-A1@admin:~$ cat mpicollect.c
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define N 10

int main()
{
    int rank, size;
    int arr[N];
    int sub_arr[N];
    int number;
    int local_count = 0, total_count = 0;
    int elements_per_process;


    double start_time, end_time, execution_time;

    MPI_Init(NULL,NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    elements_per_process = N / size;

    if(rank==0){
       /* printf("Enter number to find frequency: ");*/
        scanf("%d", &number);

        printf("Generated Array: ");
        for (int i = 0; i < N; i++)
        {
            arr[i] = rand() % 10;
            printf("%d ", arr[i]);
        }
        printf("\n");
    }

    // Synchronize all processes before timing
//    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();

    // Broadcast number to all processes
    MPI_Bcast(&number, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Scatter array elements
    MPI_Scatter(arr, elements_per_process, MPI_INT,
                sub_arr, elements_per_process, MPI_INT,
                0, MPI_COMM_WORLD);

    printf("Process : %d\n",rank);
    // Each process counts frequency
    for (int i = 0; i < elements_per_process; i++)
    {
        printf("%d\t",sub_arr[i]);
        if (sub_arr[i] == number)
            local_count++;
    }

    printf("\n");
    // Reduce counts
    MPI_Reduce(&local_count, &total_count, 1, MPI_INT,
               MPI_SUM, 0, MPI_COMM_WORLD);

  //  MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();

    execution_time = end_time - start_time;

    printf("Process: %d | Frequency: %d | Execution Time: %f seconds\n",rank,local_count,execution_time);

    if (rank == 0)
    {
        printf("Total Frequency of %d = %d\n", number, total_count);
    }

    MPI_Finalize();
    return 0;
}

III-A1@admin:~$ mpicc -g -Wall -o ex3 mpicollect.c
III-A1@admin:~$ mpiexec -n 5 ./ex3
3
Process : 1
7       5
Process: 1 | Frequency: 0 | Execution Time: 1.728512 seconds
Process : 2
3       5
Process: 2 | Frequency: 1 | Execution Time: 1.728460 seconds
Process : 3
6       2
Process: 3 | Frequency: 0 | Execution Time: 1.728498 seconds
Process : 4
9       1
Process: 4 | Frequency: 0 | Execution Time: 1.728508 seconds
Generated Array: 3 6 7 5 3 5 6 2 9 1
Process : 0
3       6
Process: 0 | Frequency: 1 | Execution Time: 0.000079 seconds
Total Frequency of 3 = 2

