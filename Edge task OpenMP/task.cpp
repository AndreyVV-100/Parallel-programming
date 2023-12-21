#include <cmath>
#include <vector>
#include <iostream>
#include <cstring>
#include <chrono>

#define NUM_THREADS 4

const unsigned m = 9;
const unsigned n = (1 << m) - 1;
const double h = 1.0 / n;

double eps = 1e-5;

void FillCurrent (std::vector<double>& a, std::vector<double>& b, std::vector<double>& c,
                  std::vector<double>& f, std::vector<double>& y_prev, double param)
{
    a[0] = 0;
    b[0] = 1;
    c[0] = 0;
    f[0] = 1;

    #pragma omp parallel for num_threads(NUM_THREADS)
    for (int j = 1; j < n - 1; ++j)
    {
        double e_y_minus = std::exp (y_prev[j - 1]);
        double e_y = std::exp (y_prev[j]);
        double e_y_plus = std::exp (y_prev[j + 1]);

        a[j] = 1.0 - h * h / 12.0 * e_y_plus;
        b[j] = -2.0 - 5.0 * h * h / 6.0 * e_y;
        c[j] = 1.0 - h * h / 12 * e_y_minus;

        f[j] = h * h / 12.0 * (e_y_plus * (1 - y_prev[j + 1]) + 10 * e_y * (1 - y_prev[j]) + e_y_minus * (1 - y_prev[j - 1]));
    }

    a[n - 1] = 0;
    b[n - 1] = 1;
    c[n - 1] = 0;
    f[n - 1] = param;
}

void Solve (std::vector<double>& a, std::vector<double>& b, std::vector<double>& c,
            std::vector<double>& f, std::vector<double>& x)
{
	#pragma omp parallel num_threads(NUM_THREADS)
    {
        unsigned stride = 1;
        for(unsigned nn = n, low = 2; nn > 1; nn /= 2, low *= 2, stride *= 2) {
            #pragma omp for
            for(int i = low - 1; i < n; i += stride * 2) {
                float alpha = -a[i] / b[i - stride];
                float gamma = -c[i] / b[i + stride];
                a[i] = alpha * a[i - stride];
                b[i] = alpha * c[i - stride] + b[i] + gamma * a[i + stride];
                c[i] = gamma * c[i + stride];
                f[i] = alpha * f[i - stride] + f[i] + gamma * f[i + stride];
            }
        }

        #pragma omp barrier

        x[n / 2] = f[n / 2] / b[n / 2];
        for(stride /= 2; stride >= 1; stride /= 2) {
            #pragma omp for
            for(unsigned i = stride - 1; i < n; i += stride * 2) {
                x[i] = (f[i] - ((i > stride) ? (a[i] * x[i - stride]) : 0.0) - ((i + stride < n) ? (c[i] * x[i + stride]) : 0.0)) / b[i];
            }
        }
    }
}

double diff (std::vector<double>& y1, std::vector<double>& y2)
{
	double res = 0.0;

	for (int i = 0; i < n; ++i)
		res += std::abs(y1[i] - y2[i]);

	return res;
}

int main(int argc, char* argv[])
{
    const int PARAM_NUM = 11;
    const double PARAM_START = 0;
    const double PARAM_STEP = 0.1;
    const unsigned NUM_ITER = 5000;

	std::vector<double> a(n), b(n), c(n), x(n);
	for (int i = 0; i < n; ++i)
		x[i] = i * h;

	std::vector<double> y_prev(n), y_cur(n), f(n);

	double param = PARAM_START;
	for (int rightBoundNum = 0; rightBoundNum < PARAM_NUM; rightBoundNum++, param += PARAM_STEP)
	{
		// start approximation: y = 1 + (b - 2) * x + x^2
		for (int i = 0; i < n; ++i)
			y_prev[i] = 1 + (param - 2) * x[i] + x[i] * x[i];

		auto time_start = std::chrono::high_resolution_clock::now();

		int i = 0;
		for (i = 0; i < NUM_ITER; ++i)
		{
			FillCurrent(a, b, c, f, y_prev, param);
			Solve(a, b, c, f, y_cur);

			if (diff (y_cur, y_prev) < eps)
				break;

            y_prev = y_cur;
		}

		auto time_end = std::chrono::high_resolution_clock::now();
        std::cout << std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_start).count() << "\n";

		std::cout << param << " ";
		for (int i = 0; i < n; ++i)
			std::cout << y_prev[i] << " ";

		std::cout << "\n";
	}
}