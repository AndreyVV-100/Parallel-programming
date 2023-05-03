#include <mpi.h>
#include <iostream>

int main (int argc, char** argv)
{
    int size = 0, rank = 0;
    MPI_Init (&argc, &argv);
    MPI_Comm_size (MPI_COMM_WORLD, &size);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);

    std::cout << "Hello, world!" << std::endl
              << "Size = " << size << std::endl
              << "Rank = " << rank << std::endl;

    MPI_Finalize();
}