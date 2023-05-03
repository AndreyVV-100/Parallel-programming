#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <atomic>
#include <cmath>

size_t StrToNum (const char* str)
{
    std::stringstream sstream(str);
    size_t size = 0;
    sstream >> size;
    return size;
}

int main (int argc, char** argv)
{
    if (argc < 3)
        return 0;

    size_t size = StrToNum(argv[1]),
              N = StrToNum(argv[2]);
    std::vector<std::atomic<double>> thread_results(size);
    for (auto& val: thread_results)
        val = NAN;
    size_t step = N / size;

    auto run_process = [size, N, step, &thread_results](size_t rank)
    {
        double tmp_res = 0.0;
        size_t start  = step * rank,
               finish = (size - 1 != rank) ? start + step : N;
        for (size_t n = start + 1; n <= finish; n++)
            tmp_res += 1.0 / n;
        thread_results[rank] = tmp_res;
    };
    for (size_t rank = 0; rank < size; rank++)
        std::thread(run_process, rank).detach();

    double common_result = 0.0;
    for (auto& thread: thread_results)
    {
        while (std::isnan(thread)) {}
        common_result += thread;
    }
    std::cout << common_result << std::endl;
    return 0;
}
