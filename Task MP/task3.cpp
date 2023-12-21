#include <mpi.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <chrono>
#include <iostream>
#include <fstream>

#define ISIZE 10000
#define JSIZE 10000

double a[ISIZE][JSIZE],
       b[ISIZE][JSIZE];

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
        part((ISIZE - 1) / size),
        exec_start(rank * part),
        exec_end(rank < size - 1 ? (rank + 1) * part : ISIZE - 1) {}
};

void SequentialExecution()
{
    for (int i=0; i<ISIZE; i++){
        for (int j = 0; j < JSIZE; j++){
            a[i][j] = sin(0.1*a[i][j]);
        }
    }
    for (int i=0; i<ISIZE-1; i++){
        for (int j = 0; j < JSIZE; j++){
            b[i][j] = a[i+1][j]*1.5;
        }
    }
}

void ParallelExecution(ExecPart part)
{
    for (int i = part.exec_start; i < part.exec_end; i++) {
        for (int j = 0; j < JSIZE; j++){
            b[i][j] = sin(0.1*a[i+1][j])*1.5;
        }
    }
}

void Print (ExecPart part)
{
    int tmp = 0;
    if (part.rank)
        MPI_Recv (&tmp, 1, MPI_INT, part.rank - 1, MPI_ANY_TAG,
                  MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    auto mode = part.rank ? std::ios_base::app : std::ios_base::out;
    std::fstream file("result3.txt", mode);
    for (int i = part.exec_start; i < part.exec_end; i++) {
        for (int j=0; j < JSIZE; j++){
            file << b[i][j] << " ";
        }
        file << std::endl;
    }
    if (part.rank < part.size - 1)   
        MPI_Send (&tmp, 1, MPI_INT, part.rank + 1, 0, MPI_COMM_WORLD);
    else {
        for (int j=0; j < JSIZE; j++){
            file << b[ISIZE - 1][j] << " ";
        }
        file << std::endl;
    }
    file.close();
}

int main (int argc, char **argv)
{
    MPI_Shell shell{&argc, &argv};
    auto [size, rank] = shell.GetSizeRank();

    for (int i=0; i<ISIZE; i++){
        for (int j=0; j<JSIZE; j++){
            a[i][j] = 10*i +j;
            b[i][j] = 0;
        }
    }
    ExecPart part(size, rank);

    //начало измерения времени
    auto time_start = std::chrono::high_resolution_clock::now();
    #ifdef NO_PARALLEL
        SequentialExecution();
    #else
        ParallelExecution (part);
    #endif
    auto time_end = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_start).count() << "\n";

    //окончание измерения времени
    MPI_Barrier(MPI_COMM_WORLD);
    Print (part);
}