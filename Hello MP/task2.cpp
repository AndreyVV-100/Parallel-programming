#include <iostream>

int main()
{
    double common_sum = 0, sum = 0;
    const int N = 100;
    #pragma omp parallel num_threads(12) private(sum)
    {
        #pragma omp for
        for (int i = 1; i <= N; i++)
            sum += 1.0 / i;
        #pragma omp critical
        common_sum += sum;
    }
    std::cout << common_sum << std::endl;
}
