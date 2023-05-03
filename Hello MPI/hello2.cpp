#include <mpi.h>
#include <iostream>
#include <sstream>

int main (int argc, char** argv)
{
    if (argc < 2)
        return 0;

    std::stringstream sstream(argv[1]);
    size_t N = 0;
    sstream >> N;

    int size = 0, rank = 0;
    MPI_Init (&argc, &argv);
    MPI_Comm_size (MPI_COMM_WORLD, &size);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    double res = 0.0;
    
    size_t step = N / size;
    size_t start  = step * rank,
           finish = (size - 1 != rank) ? start + step : N;
    for (size_t n = start + 1; n <= finish; n++)
        res += 1.0 / n;
    
    // std::cout << rank << " " << res << std::endl;
    if (rank)
        MPI_Send(&res, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    else
    {
        for (int i_rank = 1; i_rank < size; i_rank++)
        {
            double buf = 0.0;
            MPI_Recv (&buf, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG,
                      MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            res += buf;
        }
        std::cout << res << std::endl;
    }

    MPI_Finalize();
}