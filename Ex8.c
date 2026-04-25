#include <stdio.h>
#include <stdlib.h>
#include <omp.h> // Required for omp_get_wtime()

#define N 100000

int main() {
    double *A = (double *)malloc(N * sizeof(double));
    double *B = (double *)malloc(N * sizeof(double));
    double *C = (double *)malloc(N * sizeof(double));

    if (A == NULL || B == NULL || C == NULL) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    // Initialize arrays
    for (int i = 0; i < N; i++) {
        A[i] = i;
        B[i] = 2*i;
    }

    // Start timing
    double start_time = omp_get_wtime();

    // Parallel addition
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        C[i] = A[i] + B[i];
    }

    // End timing
    double end_time = omp_get_wtime();
    double duration = end_time - start_time;

    // Print first 5 elements for verification
    printf("Index | A[i] + B[i] = C[i]\n");
    for (int i = 0; i < 5; i++) {
        printf("%5d | %4.1f + %4.1f = %4.1f\n", i, A[i], B[i], C[i]);
    }

    // Print the execution time
    printf("\nParallel Addition took %f seconds.\n", duration);

    free(A); free(B); free(C);
    return 0;
}

[23bcs013@mepcolinux ex8]$./a.out
Index | A[i] + B[i] = C[i]
    0 |  0.0 +  0.0 =  0.0
    1 |  1.0 +  2.0 =  3.0
    2 |  2.0 +  4.0 =  6.0
    3 |  3.0 +  6.0 =  9.0
    4 |  4.0 +  8.0 = 12.0

Parallel Addition took 0.052054 seconds.
[23bcs013@mepcolinux ex8]$cat const.c
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define CONSTANT 5.0

void process_array(long n) {
    // Memory Allocation
    double *A = (double *)malloc(n * sizeof(double));
    if (A == NULL) return;

    // Initialization
    for (long i = 0; i < n; i++) {
        A[i] = (double)i;
    }

    // Parallel Constant Addition & Timing
    double start = omp_get_wtime();

    #pragma omp parallel for schedule(static)
    for (long i = 0; i < n; i++) {
        A[i] += CONSTANT;
    }

    double end = omp_get_wtime();

    // Output Results using Tabs
    printf("Size:\t%ld\n", n);
    printf("Time:\t%f seconds\n", end - start);

    // Print 3 indices: Start, Middle, and End using tabs
    long indices[] = {0, n / 2, n - 1};
    printf("Index\tOriginal\tConstant\tResult\n");
    for (int k = 0; k < 3; k++) {
        long idx = indices[k];
        printf("%ld\t%.1f\t\t+ %.1f\t\t= %.1f\n", idx, (double)idx, CONSTANT, A[idx]);
    }
    printf("\n"); // New line for next size block

    free(A);
}

int main() {
    long sizes[] = {10000, 1000000, 10000000};

    for (int i = 0; i < 3; i++) {
        process_array(sizes[i]);
    }

    return 0;
}

[23bcs013@mepcolinux ex8]$cc -std=c99 -fopenmp const.c
[23bcs013@mepcolinux ex8]$./a.out
Size:   10000
Time:   0.021766 seconds
Index   Original        Constant        Result
0       0.0             + 5.0           = 5.0
5000    5000.0          + 5.0           = 5005.0
9999    9999.0          + 5.0           = 10004.0

Size:   1000000
Time:   0.024714 seconds
Index   Original        Constant        Result
0       0.0             + 5.0           = 5.0
500000  500000.0        + 5.0           = 500005.0
999999  999999.0        + 5.0           = 1000004.0

Size:   10000000
Time:   0.051342 seconds
Index   Original        Constant        Result
0       0.0             + 5.0           = 5.0
5000000 5000000.0       + 5.0           = 5000005.0
9999999 9999999.0       + 5.0           = 10000004.0


