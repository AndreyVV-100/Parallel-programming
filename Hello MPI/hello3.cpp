#include <mpi.h>
#include <iostream>

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
    if (size == 1)   
        return 0;

    int var = 42;
    if (rank == 0)
    {
        std::cout << "From [0] send to [1]: " << var << std::endl;
        MPI_Send (&var, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Recv (&var, 1, MPI_INT, size - 1, MPI_ANY_TAG,
                  MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "[0] received from [" << size - 1 << "]:" << var << std::endl;
    }
    else
    {
        MPI_Recv (&var, 1, MPI_INT, rank - 1, MPI_ANY_TAG,
                  MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "[" << rank << "] received from [" << rank - 1 << "]:" << var << std::endl;
        var++;
        std::cout << "From [" << rank <<"] send to [" << rank + 1 <<"]: " << var << std::endl;
        MPI_Send (&var, 1, MPI_INT, (rank + 1) % size, 0, MPI_COMM_WORLD);
    }
}