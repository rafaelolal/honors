#include <chrono>
#include <cstdint>
#include <cuda_runtime.h>
#include <iostream>
#include <limits>

#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <thrust/copy.h>
#include <thrust/device_vector.h>
#include <thrust/fill.h>
#include <thrust/host_vector.h>
#include <thrust/reduce.h>
// #include <thrust/reduce_by_key.h>
#include <thrust/sort.h>
#include <thrust/tabulate.h>
#include <thrust/transform.h>

#include <cub/device/device_reduce.cuh>
#include <thrust/iterator/constant_iterator.h>
#include <thrust/iterator/counting_iterator.h>
#include <thrust/iterator/transform_iterator.h>
#include <thrust/tuple.h>

using namespace std;

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cout
        << "argument (int) E (for experiment number) and (int) N is required!";
    return 0;
  }

  unsigned long long N = stoull(argv[2]);
  string experiment_number = argv[1];

  auto start = chrono::steady_clock::now();

  auto first = thrust::make_counting_iterator<unsigned long long>(1ULL);
  auto last = first + N;

  using Aggregate =
      thrust::tuple<unsigned long long, unsigned long long, unsigned long long>;

  unsigned long long X_LIMIT =
      (numeric_limits<unsigned long long>::max() - 1) / 3;
  auto collatz = [X_LIMIT] __device__(unsigned long long x) -> Aggregate {
    unsigned int steps = 0;
    unsigned long long peak = x;

    while (x != 1) {
      if (x % 2 == 0) {
        x = x / 2;

      } else if (x > X_LIMIT) {
        return thrust::make_tuple(0U, 0ULL, 1ULL);
      } else {
        x = 3 * x + 1;
      }

      peak = peak > x ? peak : x;
      steps++;
    }

    return thrust::make_tuple(steps, peak, 0ULL);
  };

  auto combine = [] __device__(Aggregate a, Aggregate b) -> Aggregate {
    return thrust::make_tuple(thrust::get<0>(a) + thrust::get<0>(b),
                              thrust::get<1>(a) > thrust::get<1>(b)
                                  ? thrust::get<1>(a)
                                  : thrust::get<1>(b),
                              thrust::get<2>(a) + thrust::get<2>(b));
  };

  Aggregate total = thrust::transform_reduce(
      first, last, collatz, thrust::make_tuple(0ULL, 0ULL, 0ULL), combine);

  auto step_sum = thrust::get<0>(total);
  auto max_peak = thrust::get<1>(total);
  auto failure_count = thrust::get<2>(total);

  auto computation_done = chrono::steady_clock::now();
  auto elapsed = chrono::duration<double>(computation_done - start);

  ofstream summary("summary_" + experiment_number + ".txt");

  if (!summary) {
    cerr << "Could not open file!";
    return 1;
  }

  summary << "Computation time: " << elapsed.count() << " seconds\n";
  auto writing_done = chrono::steady_clock::now();
  summary << "N=" << N << '\n';
  summary << "failure_count=" << failure_count << '\n';
  summary << "sum_stopping_times=" << step_sum << '\n';
  summary << "average_stopping_time=" << step_sum / N << '\n';
  summary << "maximum_peak=" << max_peak << '\n';

  summary.close();

  elapsed = chrono::duration<double>(writing_done - computation_done);
  cout << "Writing time: " << elapsed.count() << " seconds\n";

  return 0;
}