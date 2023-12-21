#include <iostream>
#include <omp.h>

int main()
{
    int shared = 0;
    #pragma omp parallel num_threads(12)
    #pragma omp critical
    {
        shared++;
        std::cout << omp_get_thread_num() << " " << shared << std::endl;
    }
}
