#include <mpi.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <chrono>
#include <iostream>
#include <fstream>

#define ISIZE 10000
#define JSIZE 10000

double a[ISIZE][JSIZE];

struct MPI_Shell
{
    MPI_Shell (int* argc, char*** argv)
    {
        MPI_Init (argc, argv);
    }

    ~MPI_Shell()
    {
        MPI_Finalize();
    }

    std::pair<int, int> GetSizeRank()
    {
        int size = 0, rank = 0;
        MPI_Comm_size (MPI_COMM_WORLD, &size);
        MPI_Comm_rank (MPI_COMM_WORLD, &rank);
        return {size, rank};
    }
};

struct ExecPart
{
    int size, rank, part, exec_start, exec_end;
    ExecPart(int size, int rank):
        size(size),
        rank(rank),
        part((ISIZE - 2) / size),
        exec_start(rank * part + 2),
        exec_end(rank < size - 1 ? (rank + 1) * part + 2: ISIZE) {}
};

void SequentialExecution()
{
    for (int i = 1; i < ISIZE; i++) {
        for (int j = 3; j < JSIZE; j++) {
            a[i][j] = sin(2*a[i-1][j-3]);
        }
    }
}

// I hate copypaste, but...
void RunSeqPart(int i, int j)
{
    while (i < ISIZE && j < JSIZE) {
        a[i][j] = sin(2*a[i-1][j-3]);
        i += 1;
        j += 3;
    }
}

void SendSeqPart(int i, int j)
{
    while (i < ISIZE && j < JSIZE) {
        MPI_Send(&a[i][j], 1, MPI_DOUBLE, 0, i * JSIZE + j, MPI_COMM_WORLD);
        i += 1;
        j += 3;
    }
}

void RecvSeqPart(int i, int j)
{
    while (i < ISIZE && j < JSIZE) {
        MPI_Recv(&a[i][j], 1, MPI_DOUBLE, MPI_ANY_SOURCE, i * JSIZE + j, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        i += 1;
        j += 3;
    }
}

void ParallelExecution(ExecPart part)
{
    int i_base = 1, j_base = 3;
    if (part.rank == 0)
        for (; j_base < JSIZE; j_base++)
            RunSeqPart(i_base, j_base);
    for (i_base = part.exec_start; i_base < part.exec_end; i_base++)
        for (j_base = 3; j_base < 6; j_base++)
            RunSeqPart(i_base, j_base);
}

void RecvFromOtherThreads(ExecPart part)
{
    int tmp = 0;
    MPI_Send (&tmp, 1, MPI_INT, part.rank + 1, 0, MPI_COMM_WORLD);
    for (int i_base = part.exec_end; i_base < ISIZE; i_base++)
        for (int j_base = 3; j_base < 6; j_base++)
            RecvSeqPart(i_base, j_base);
}

void SendToMainThread(ExecPart part)
{
    int tmp = 0;
    MPI_Recv (&tmp, 1, MPI_INT, part.rank - 1, MPI_ANY_TAG,
                  MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    for (int i_base = part.exec_start; i_base < part.exec_end; i_base++)
        for (int j_base = 3; j_base < 6; j_base++)
            SendSeqPart(i_base, j_base);
    if (part.rank < part.size - 1)   
        MPI_Send (&tmp, 1, MPI_INT, part.rank + 1, 0, MPI_COMM_WORLD);
}

void Print()
{
    std::fstream file("result1.txt", std::ios_base::out);
    for (int i = 0; i < ISIZE; i++) {
        for (int j=0; j < JSIZE; j++){
            file << a[i][j] << " ";
        }
        file << std::endl;
    }
    file.close();
}

int main(int argc, char **argv)
{
    MPI_Shell shell(&argc, &argv);
    auto [size, rank] = shell.GetSizeRank();
    ExecPart part(size, rank);
    // FILE *ff;
    for (int i=0; i<ISIZE; i++){
        for (int j=0; j<JSIZE; j++){
            a[i][j] = 10*i +j;
        }
    }

    //начало измерения времени
    auto time_start = std::chrono::high_resolution_clock::now();
    #ifdef NO_PARALLEL
        SequentialExecution();
    #else
        ParallelExecution(part);
    #endif
    auto time_end = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_start).count() << "\n";

    #ifndef NO_PARALLEL
    if (rank == 0)
        RecvFromOtherThreads(part);
    else
        SendToMainThread(part);
    #endif
    // if (rank == 0)
    //     Print();
    //окончание измерения времени
    // Print (part);

    // //окончание измерения времени
    // ff = fopen("result.txt","w");
    // for(i= 0; i < ISIZE; i++){
    //     for (j= 0; j < JSIZE; j++){
    //         fprintf(ff,"%f ",a[i][j]);
    //     }
    //     fprintf(ff,"\n");
    // }
    // fclose(ff);
}