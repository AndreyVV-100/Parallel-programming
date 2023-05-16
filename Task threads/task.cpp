#include <iostream>
#include <chrono>
#include <stack>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <cmath>
#include <mutex>
#include <vector>
#include <sstream>
#include <iomanip>

using FuncType = double (*) (double);

template <typename T>
T GetFromString (const char* str)
{
    std::stringstream sstream(str);
    T elem;
    sstream >> elem;
    return elem;
}

inline double GetSquare(double A, double B, double fA, double fB)
{
    return (fA + fB) * (B - A) / 2;
}

struct Interval
{
    double A, B, fA, fB, S;

    Interval() = default;

    Interval (double start, double end, double f_start, double f_end, double Square):
        A (start),
        B (end),
        fA (f_start),
        fB (f_end),
        S (Square)
    {}

    Interval (double start, double end, FuncType f):
        A (start),
        B (end),
        fA (f (A)),
        fB (f (B)),
        S (GetSquare (A, B, fA, fB))
    {}

    std::pair <Interval, Interval> GetHalfs (FuncType f)
    {
        double mid = (A + B) / 2,
               f_mid = f (mid);
        return {Interval(A, mid, fA, f_mid, GetSquare (A, mid, fA, f_mid)),
                Interval(mid, B, f_mid, fB, GetSquare (mid, B, f_mid, fB))};
    }
};

namespace test
{

double f (double x)
{
    return sin (1 / (x - 1) / (x - 1));
}

constexpr double x_start = 0.0;
constexpr double x_end = 0.99;

}

class Integrator
{
    const FuncType f_;
    const Interval interval_;
    const double eps_;

    std::atomic<double> result_ = 0.0;
    std::stack<Interval> stack_;

    std::mutex mutex_;
    std::condition_variable cond_var_;
    std::atomic<size_t> barrier_ = 0;
    size_t n_threads_ = 0;
    std::atomic<bool> empty_stack = true;

public:

    Integrator (double start, double end, FuncType f, double precision):
        f_ (f),
        interval_ (start, end, f),
        eps_ (precision)
    {}

    double Calculate (size_t n_threads)
    {
        n_threads_ = n_threads;
        result_ = 0.0;
        stack_.push (interval_);

        std::vector<std::thread> thread_pool;
        for (size_t i_thread = 0; i_thread < n_threads; i_thread++)
            thread_pool.emplace_back([this](){CalculateThread();});

        int counter = 0;
        for (auto& thread: thread_pool) {
            // if (vec.joinable())
                thread.join(); counter++;}
        return result_;
    }

private:

    void CalculateThread()
    {
        std::stack<Interval> local_stack;
        barrier_++;
        while (true)
        {
            std::unique_lock lock(mutex_);
            cond_var_.wait (lock, [this](){return !stack_.empty() || barrier_ == n_threads_;});
            if (stack_.empty())
            {
                cond_var_.notify_all();
                break;
            }
            barrier_--;

            Interval interval = stack_.top(), left, right;
            stack_.pop();
            empty_stack = stack_.empty();
            lock.unlock();

            while (true)
            {
                std::tie (left, right) = interval.GetHalfs(f_);
                double S = left.S + right.S;
                if (std::abs (interval.S - S) < eps_ * std::abs (S))
                {
                    result_.fetch_add (S);
                    if (local_stack.empty())
                    {
                        barrier_++;
                        cond_var_.notify_all();
                        break;
                    }

                    interval = local_stack.top();
                    local_stack.pop();
                    if (empty_stack)
                    {
                        std::lock_guard guard(mutex_);

                        while (!local_stack.empty())
                        {
                            stack_.push(local_stack.top());
                            local_stack.pop();
                        }
                        empty_stack = false;
                    }
                }
                else
                {
                    interval = left;
                    local_stack.push (right);
                }
            }
        }
    }
};

int main (int argc, char** argv)
{
    if (argc < 3)
        return 0;

    size_t n_threads = GetFromString<size_t> (argv[1]);
    double eps = GetFromString<double> (argv[2]);

    auto time_start = std::chrono::system_clock::now();
    Integrator integrator (test::x_start, test::x_end, test::f, eps);
    double result = integrator.Calculate(n_threads);
    auto time_end = std::chrono::system_clock::now();

    std::cout << std::setprecision(10) << result << ", time = " <<
                 std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_start).count() << "\n";
    return 0;
}