#include <chrono>
#include <climits>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <thrust/execution_policy.h>
#include <thrust/iterator/counting_iterator.h>
#include <thrust/random.h>
#include <thrust/transform.h>
#include <vector>

using namespace std;

__global__ void mmult(int* A, int* B, int* C, int width) {

  // this thread computes C[row,col]

  // find (row,col) in the matrix
  int row = blockIdx.y * blockDim.y + threadIdx.y;
  int col = blockIdx.x * blockDim.x + threadIdx.x;

  if ((row < width) && (col < width)) {
    int c = 0;
    for (int k = 0; k < width; k++) {
      c += A[row * width + k] * B[k * width + col];
    }
    C[row * width + col] = c;
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    cout << "argument (int) E (for experiment number) and (int) N is required!";
    return 0;
  }

  int N = stoi(argv[2]);
  string experiment_number = argv[1];
  size_t size = (size_t)N * N * sizeof(int);

  int *A_d, *B_d, *C_d;
  cudaMalloc((void**)&A_d, size);
  cudaMalloc((void**)&B_d, size);
  cudaMalloc((void**)&C_d, size);

  /////////////////////////////////////////
  // fill in the matrices with random numbers
  // Filling the matrices with a parallel algorithm so that it's fast
  // Normally, the matrix would be read from disk and then passed to the device
  // instead of being directly created in the device like it is here
  auto start = chrono::high_resolution_clock::now();

  auto start_index = thrust::make_counting_iterator(0);
  auto end_index = start_index + N * N;
  thrust::transform(
      thrust::device, start_index, end_index, A_d, [] __device__(int x) {
        thrust::default_random_engine rng(x);
        thrust::uniform_int_distribution<int> distr(INT_MIN, INT_MAX);
        return distr(rng);
      });

  // offset the seed so that B is not identical to A
  int offset = N * N;
  thrust::transform(
      thrust::device, start_index, end_index, B_d, [offset] __device__(int x) {
        thrust::default_random_engine rng(x + offset);
        thrust::uniform_int_distribution<int> distr(INT_MIN, INT_MAX);
        return distr(rng);
      });

  // kernels are asynchronous, wait for them before stopping the clock
  cudaDeviceSynchronize();
  auto gen_done = chrono::high_resolution_clock::now();
  auto time_to_gen = chrono::duration<double>(gen_done - start).count();

  cout << "time_to_gen " << time_to_gen << "\n";

  ///////////////////////////////////
  // compute A x B
  // gridDim.x : 1 to 2³¹ − 1 gridDim.y : 1 to 65, 535 gridDim.z : 1 to 65,535
  // Round up division...
  dim3 gridDim((N + 15) / 16, (N + 15) / 16);
  // blockDim.x × blockDim.y × blockDim.z ≤ 1024
  // blockDim.x ≤ 1024 blockDim.y ≤ 1024 blockDim.z ≤ 64
  dim3 blockDim(16, 16);
  // Chapter 4 stuff:
  // A thread block is assigned entirely to one Streaming Multiprocessor (SM)
  // Could have multiple thread blocks in one SM if resources are available
  // blocks are scheduled into warps of 32 threads that the SM then executes

  mmult<<<gridDim, blockDim>>>(A_d, B_d, C_d, N);
  cudaDeviceSynchronize();
  auto mult_done = chrono::high_resolution_clock::now();
  auto time_to_mult = chrono::duration<double>(mult_done - gen_done).count();

  cout << "time_to_mult " << time_to_mult << "\n";

  // copy the result back to the host (not part of the timed multiplication,
  // the serial version has no equivalent step)
  vector<int> C_h((size_t)N * N);
  cudaMemcpy(C_h.data(), C_d, size, cudaMemcpyDeviceToHost);

  cudaFree(A_d);
  cudaFree(B_d);
  cudaFree(C_d);

  ////////////////////////////////////////
  // print stats
  ofstream summary("summary_" + experiment_number + ".txt");

  if (!summary) {
    cerr << "Could not open file!";
    return 1;
  }

  summary << "Computation time: " << time_to_mult << " seconds\n";
  summary << "N=" << argv[2] << '\n';
  summary << "time_to_gen=" << time_to_gen << '\n';

  summary.close();

  return 0;
}
