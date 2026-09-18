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

#include <thrust/iterator/constant_iterator.h>
#include <thrust/iterator/counting_iterator.h>
#include <thrust/iterator/transform_iterator.h>

#include <cub/device/device_reduce.cuh>

using namespace std;

__device__ unsigned long long X_LIMIT =
    (numeric_limits<unsigned long long>::max() - 1) / 3;
__device__ unsigned long long step_sum = 0;
__device__ unsigned int failure_count = 0;
__device__ unsigned long long max_peak = 0;

struct Result {
  unsigned int steps = 0;
  unsigned long long peak = 0;
};

struct AnalyzeCollatz {
  __device__ Result operator()(unsigned long long x) {
    unsigned int steps = 0;
    unsigned long long peak = x;
    while (x != 1) {
      if (x % 2 == 0) {
        x = x / 2;

      } else if (x > X_LIMIT) {
        // May have to acquire a lock to do this
        // Could refactor this to be a reduce over the array of results
        // will it have better run time?
        atomicAdd(&failure_count, 1);
        return {};
      } else {
        x = 3 * x + 1;
      }

      peak = max(peak, x);
      steps++;
    }

    // Same as failure_count, do I need a lock and should I just reduce this
    // later?
    atomicAdd(&step_sum, (unsigned long long)(steps));
    max_peak = atomicMax(&max_peak, peak);
    return Result{steps, peak};
  }
};

int main(int argc, char* argv[]) {
  if (argc != 3) {
    cout << "argument (int) E (for experiment number) and (int) N is required!";
    return 0;
  }

  auto start = chrono::steady_clock::now();

  unsigned int N = stoul(argv[2]);
  auto start_x = thrust::make_counting_iterator<unsigned long long>(1);
  auto last_x = start_x + N;
  thrust::device_vector<unsigned long long> d_values(N);
  thrust::host_vector<unsigned long long> h_values(N);

  thrust::device_vector<Result> d_results(N);
  thrust::host_vector<Result> h_results(N);

  AnalyzeCollatz collatz;
  thrust::transform(start_x, last_x, d_results.begin(), collatz);

  h_results = d_results;

  auto computation_done = chrono::steady_clock::now();
  auto elapsed = chrono::duration<double>(computation_done - start);

  ofstream summary("summary_" + string(argv[1]) + ".txt");

  if (!summary) {
    cerr << "Could not open file!";
    return 1;
  }

  unsigned long long h_failure_count;
  cudaMemcpyFromSymbol(&h_failure_count, failure_count, sizeof(failure_count),
                       0, cudaMemcpyDeviceToHost);

  unsigned long long h_step_sum;
  cudaMemcpyFromSymbol(&h_step_sum, step_sum, sizeof(step_sum), 0,
                       cudaMemcpyDeviceToHost);

  unsigned long long h_max_peak;
  cudaMemcpyFromSymbol(&h_max_peak, max_peak, sizeof(max_peak), 0,
                       cudaMemcpyDeviceToHost);

  summary << "Computation time: " << elapsed.count() << " seconds\n";
  auto writing_done = chrono::steady_clock::now();
  summary << "failure_count=" << h_failure_count << '\n';
  summary << "sum_stopping_times=" << h_step_sum << '\n';
  summary << "average_stopping_time=" << h_step_sum / N << '\n';
  summary << "maximum_peak=" << h_max_peak << '\n';

  summary.close();

  elapsed = chrono::duration<double>(writing_done - computation_done);
  cout << "Writing time: " << elapsed.count() << " seconds\n";

  return 0;
}