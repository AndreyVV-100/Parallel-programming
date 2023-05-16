#include <mpi.h>
#include <iostream>
#include <chrono>
#include <cmath>
#include <cassert>
#include <fstream>

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

struct Task
{
    /*
        du/dt + a*du/dx = f(x,t)
        u(x,0) = phi(x)
        u(0,t) = psi(t)
    */
    using fType = double (*) (double x, double t);
    using phiType = double (*) (double);
    using psiType = phiType;

    double a;
    fType f;
    phiType phi;
    psiType psi;
    double x_start, x_end, t_start, t_end;

    void RecountBounds (int rank, int size)
    {
        double x_chunk = (x_end - x_start) / size;
        x_start += x_chunk * rank;
        x_end = x_start + x_chunk;
    }
};

namespace test
{

double f (double x, double t)
{
    return x + t;
}

double phi (double x)
{
    return cos (M_PI * x);
}

double psi (double t)
{
    return exp (-t);
}

constexpr Task task {2.0, f, phi, psi, 0.0, 1.0, 0.0, 1.0};

} // namespace test

template <typename T>
class Array2D
{
    T* array_ = nullptr;
    size_t width_ = 0, height_ = 0;

public:
    Array2D (size_t width, size_t height):
        width_ (width),
        height_ (height),
        array_ (new T[height * width])
    {}

    Array2D(const Array2D&) = delete;
    Array2D& operator=(const Array2D&) = delete;
    Array2D(Array2D&&) = delete;
    Array2D& operator=(Array2D&&) = delete;

    ~Array2D() {delete [] array_;}
    T& operator()(size_t x, size_t y) {return array_[y * width_ + x];}
};

class Field
{
    size_t x_size_, t_size_;
    Array2D<double> array_;
    Task task_;
    int rank_, size_;
    double h_, tau_;

public:
    
    Field (size_t x_size, size_t t_size, Task task, int rank, int size):
        x_size_ (x_size),
        t_size_ (t_size),
        array_ (x_size / size, t_size_),
        task_ (task),
        rank_ (rank),
        size_ (size)
    {
        assert (rank >= 0);
        assert (size > 0);
        assert (x_size >= size);
        // assert (!(x_size % size));

        h_ = (task_.x_end - task_.x_start) / x_size_;
        tau_ = (task_.t_end - task_.t_start) / t_size_;
        task_.RecountBounds (rank_, size_);
        x_size_ /= size_;

        for (size_t i_x = 0; i_x < x_size_; i_x++)
            array_(i_x, 0) = task_.phi(task_.x_start + h_ * i_x);
        if (!rank_)
            for (size_t i_t = 0; i_t < t_size_; i_t++)
                array_(0, i_t) = task_.psi(task_.t_start + tau_ * i_t);
    }

    void CountByLeftAngle()
    {
        double t_now = task_.t_start + tau_;
        bool first = !rank_,
             not_last = (rank_ < size_ - 1);

        for (size_t t_i = 1; t_i < t_size_; t_i++, t_now += tau_)
        {
            double u_prev = array_(0, t_i - 1);
            if (!first)
            {
                MPI_Recv (&u_prev, 1, MPI_DOUBLE, rank_ - 1, MPI_ANY_TAG,
                          MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            if (not_last)
                MPI_Send (&array_(x_size_ - 1, t_i - 1), 1, MPI_DOUBLE, rank_ + 1, 0, MPI_COMM_WORLD);

            double x_now = task_.x_start + h_ * first;
            for (size_t x_i = first; x_i < x_size_; x_i++, x_now += h_)
            {
                double u_now = array_(x_i, t_i - 1);
                array_(x_i, t_i) = u_now + tau_ * (task_.f (x_now, t_now) - 
                                   (u_now - u_prev) / h_);
                u_prev = u_now;
            }
        }
    }

    void Print()
    {
        int tmp = 0;
        if (rank_)
        {
            MPI_Recv (&tmp, 1, MPI_INT, rank_ - 1, MPI_ANY_TAG,
                          MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        auto mode = rank_ ? std::ios_base::app : std::ios_base::out;
        std::fstream file("arr.py", mode);
        if (!rank_)
            file << "arr = [";

        size_t t_step = t_size_ > 100 ? t_size_ / 100 : 1,
               x_step = x_size_ > 100 ? x_size_ / 100 : 1;

        for (size_t i_t = 0; i_t < t_size_; i_t += t_step)
        {
            for (size_t i_x = 0; i_x < x_size_; i_x += x_step)
                file << "[" << task_.x_start + h_   * i_x << ", " <<
                               task_.t_start + tau_ * i_t << ", " << 
                               array_(i_x, i_t) << "], ";
            file << "\n";
        }

        if (rank_ < size_ - 1)
        {
            file.close();
            MPI_Send (&tmp, 1, MPI_INT, rank_ + 1, 0, MPI_COMM_WORLD);
        }
        else
            file << "]";
    }
};

int main (int argc, char** argv)
{
    MPI_Shell shell{&argc, &argv};
    auto [size, rank] = shell.GetSizeRank();
    
    auto time_start = std::chrono::system_clock::now();
    Field field (18000, 60000, test::task, rank, size);
    field.CountByLeftAngle();
    auto time_end = std::chrono::system_clock::now();
    if (rank == size - 1)
        std::cout << std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_start).count() << "\n";

    field.Print();
    return 0;
}