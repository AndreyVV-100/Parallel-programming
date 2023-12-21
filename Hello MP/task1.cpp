#include <iostream>

int main()
{
    #pragma omp parallel for num_threads(12)
    for (int i = 0; i < 12; i++)
        #pragma omp critical
        std::cout << "Hello, world!\n";
}
