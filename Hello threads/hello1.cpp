#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

void RunProcess(size_t size, size_t rank)
{
    std::stringstream str_stream;
    str_stream << "Hello, world!" << std::endl
               << "Size = " << size << std::endl
               << "Rank = " << rank << std::endl;
    std::cout << str_stream.str();
}

int main (int argc, char** argv)
{
    if (argc < 2)
        return 0;

    std::stringstream sstream(argv[1]);
    size_t size = 0;
    sstream >> size;
    std::vector<std::thread> thread_pool(size);

    for (size_t rank = 0; rank < size; rank++)
        thread_pool.push_back(std::thread(RunProcess, size, rank));

    for (auto&& thread: thread_pool)
        if (thread.joinable())
            thread.join();
    return 0;
}