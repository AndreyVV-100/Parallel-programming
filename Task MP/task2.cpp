#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <chrono>
#include <iostream>
#include <fstream>
#include <atomic>

#define ISIZE 1000
#define JSIZE 100000

double a[ISIZE][JSIZE];


void SequentialExecution()
{
    for (int i=0; i<ISIZE-1; i++){
        for (int j = 6; j < JSIZE; j++){
            a[i][j] = sin(0.2*a[i+1][j-6]);
        }
    }
}

void ParallelExecution()
{
    #pragma omp parallel num_threads(12)
    for (int i=0; i<ISIZE-1; i++){
        #pragma omp for schedule(static)
        for (int j = 6; j < JSIZE; j++){
            a[i][j] = sin(0.2*a[i+1][j-6]);
        }
        #pragma omp barrier
    }
}

void Print()
{
    std::fstream file("result2.txt", std::ios_base::out);
    for (int i = 0; i < ISIZE; i++) {
        for (int j=0; j < JSIZE; j++){
            file << a[i][j] << " ";
        }
        file << std::endl;
    }
    file.close();
}

int main(int argc, char **argv)
{
    int i, j;
    FILE *ff;
    for (i=0; i<ISIZE; i++){
        for (j=0; j<JSIZE; j++){
            a[i][j] = 10*i +j;
        }
    }

    auto time_start = std::chrono::high_resolution_clock::now();
    #ifdef NO_PARALLEL
        SequentialExecution();
    #else
        ParallelExecution();
    #endif
    auto time_end = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_start).count() << "\n";

    //окончание измерения времени
    Print();
}