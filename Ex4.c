III-A1@cil26:~$ cat ex4.c
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX 100
struct Student
{
    char name[50];
    int roll;
    int marks[5];
    int total;
    char grade;
};
int get_input(struct Student students[])
{
    FILE *fp = fopen("students.txt", "r");
    int n = 0;
    if(fp == NULL)
    {
        printf("File not found\n");
        return 0;
    }
    while(fscanf(fp, "%s %d %d %d %d %d %d",
        students[n].name,
        &students[n].roll,
        &students[n].marks[0],
        &students[n].marks[1],
        &students[n].marks[2],
        &students[n].marks[3],
        &students[n].marks[4]) != EOF)
    {
            n++;
    }

    fclose(fp);
    return n;
}
void calculateResult(struct Student *s)
{
    s->total = 0;
    for(int i = 0; i < 5; i++)
        s->total += s->marks[i];
    float avg = s->total / 5.0;
    if(avg >= 90) s->grade = 'A';
    else if(avg >= 75) s->grade = 'B';
    else if(avg >= 60) s->grade = 'C';
    else if(avg >= 50) s->grade = 'D';
    else s->grade = 'F';
}
int main()
{
    int rank, size, n;
    struct Student students[MAX];
    struct Student local;
    MPI_Datatype MPI_STUDENT;
    MPI_Init(NULL,NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int blocklengths[5] = {50, 1, 5, 1, 1};
    MPI_Datatype types[5] = {MPI_CHAR, MPI_INT, MPI_INT, MPI_INT, MPI_CHAR};
    MPI_Aint displacements[5];
    struct Student temp;
    MPI_Aint base;
    MPI_Get_address(&temp, &base);
    MPI_Get_address(&temp.name, &displacements[0]);
    MPI_Get_address(&temp.roll, &displacements[1]);
    MPI_Get_address(&temp.marks, &displacements[2]);
    MPI_Get_address(&temp.total, &displacements[3]);
    MPI_Get_address(&temp.grade, &displacements[4]);
    for(int i = 0; i < 5; i++)
        displacements[i] -= base;
    MPI_Type_create_struct(5, blocklengths, displacements, types, &MPI_STUDENT);
    MPI_Type_commit(&MPI_STUDENT);
    double start_time, end_time;
    if(rank == 0)
    {
        start_time = MPI_Wtime();
        n = get_input(students);
    }
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(students, 1, MPI_STUDENT,&local, 1, MPI_STUDENT,0, MPI_COMM_WORLD);
    double process_start_time = MPI_Wtime();
    calculateResult(&local);
    double process_end_time = MPI_Wtime();
    double process_time = (process_end_time - process_start_time) * 1000;
    MPI_Gather(&local, 1, MPI_STUDENT,students, 1, MPI_STUDENT,0, MPI_COMM_WORLD);
    if(rank == 0)
    {
        FILE *out = fopen("grades.txt", "w");
        for(int i = 0; i < n; i++)
        {
            fprintf(out, "%s %d Total=%d Grade=%c\n",students[i].name,students[i].roll,students[i].total,students[i].grade);
        }
        fclose(out);
        printf("Grades written to grades.txt\n");
        end_time = MPI_Wtime();
        printf("Process 0 execution time: %f milliseconds\n", (end_time - start_time) * 1000);
    }
    double total_time;
    MPI_Reduce(&process_time, &total_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    if(rank != 0)
    {
        printf("Process %d execution time: %f milliseconds\n", rank, process_time);
    }
    MPI_Type_free(&MPI_STUDENT);
    MPI_Finalize();
    return 0;
}

III-A1@cil26:~$ mpicc -g -Wall -o exe ex4.c
III-A1@cil26:~$ mpiexec -n 6 ./exe
Process 1 execution time: 0.000152 milliseconds
Process 2 execution time: 0.000082 milliseconds
Process 3 execution time: 0.000113 milliseconds
Process 4 execution time: 0.000119 milliseconds
Process 5 execution time: 0.000126 milliseconds
Grades written to grades.txt
Process 0 execution time: 0.126800 milliseconds
III-A1@cil26:~$ cat grades.txt
Andal 1 Total=395 Grade=B
Billy 2 Total=291 Grade=D
Max 3 Total=455 Grade=A
Nancy 4 Total=309 Grade=C
Priya 5 Total=330 Grade=C
Joyce 6 Total=432 Grade=B
III-A1@cil26:~$ cat students.txt
Andal    1 60 85 80 88 82
Billy    2 60 70 82 56 23
Max      3 99 92 90 84 90
Nancy    4 49 50 60 98 52
Priya    5 78 82 74 20 76
Joyce    6 95 80 79 91 87

