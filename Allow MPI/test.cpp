#include <mpi.h>
#include <iostream>
#include <chrono>

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

int main (int argc, char** argv)
{
    MPI_Shell shell{&argc, &argv};
    auto [size, rank] = shell.GetSizeRank();
    if (size == 1 || rank > 1)   
        return 0;

    int N = 100000;
    auto time_start = std::chrono::system_clock::now();
    if (rank)
        for (int i = 0; i < N; i++)
            MPI_Send (&time_start, 1, MPI_LONG, 0, 0, MPI_COMM_WORLD);
    else
    {
        for (int i = 0; i < N; i++)
        MPI_Recv (&time_start, 1, MPI_LONG, 1, MPI_ANY_TAG,
                  MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        auto time_end = std::chrono::system_clock::now();
        std::cout << std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_start).count());
    }

    return 0;
}