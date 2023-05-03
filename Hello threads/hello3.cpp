#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>

size_t StrToNum (const char* str)
{
    std::stringstream sstream(str);
    size_t size = 0;
    sstream >> size;
    return size;
}

class ProtectedVar
{
    std::mutex mutex;
    std::condition_variable condvar;
    std::atomic<int> order = 0;
    int var = 0;

    public:
    ProtectedVar() = default;
    void ChangeVar(int tid);
};

void ProtectedVar::ChangeVar(int tid)
{
    std::unique_lock lock(mutex);
    condvar.wait(lock, [tid, &order = this->order](){return order == tid;});
    std::cout << "The thread [" << tid << "] is owning var now.\n" <<
                 "Current value is " << var << "\n" <<
                 "Changing to " << (var = (var + tid) * 2) << "\n\n";
    order.fetch_add(1);
    condvar.notify_all();
}

int main (int argc, char** argv)
{
    ProtectedVar var;
    if (argc < 2)
        return 0;
    size_t N = StrToNum (argv[1]);
    std::vector<std::thread> thread_pool;

    for (size_t i_thread = 0; i_thread < N; i_thread++)
        thread_pool.emplace_back ([&var, i_thread](){var.ChangeVar(i_thread);});
    
    for (auto& i_thread: thread_pool)
        i_thread.join();

    return 0;
}