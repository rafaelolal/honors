#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#define CUDA_CHECK(call)                                                      \
  do {                                                                        \
    cudaError_t err = (call);                                                 \
    if (err != cudaSuccess) {                                                 \
      std::fprintf(stderr, "CUDA error at %s:%d: %s\n", __FILE__, __LINE__,   \
                   cudaGetErrorString(err));                                  \
      std::exit(EXIT_FAILURE);                                                \
    }                                                                         \
  } while (0)

namespace big {

constexpr int height = 4096;
constexpr int width = 4096;
constexpr int num_frames = 50;
constexpr int steps_per_frame = 2000; // total simulation steps = 50 * 2000 = 100,000
constexpr const char *output_dir = "frames";

constexpr float low_temp = 15.0f;
constexpr float high_temp = 90.0f;

__global__ void init_kernel(float *data, int height, int width) {
  const int idx = blockIdx.x * blockDim.x + threadIdx.x;
  const int total = height * width;
  if (idx >= total) return;

  const int row = idx / width;
  data[idx] = (row == 0 || row == height - 1) ? high_temp : low_temp;
}

__global__ void simulate_kernel(const float *in, float *out, int height,
                                 int width) {
  const int idx = blockIdx.x * blockDim.x + threadIdx.x;
  const int total = height * width;
  if (idx >= total) return;

  const int row = idx / width;
  const int column = idx % width;

  if (row > 0 && column > 0 && row < height - 1 && column < width - 1) {
    const float center = in[idx];
    const float d2tdx2 = in[idx - 1] - 2.0f * center + in[idx + 1];
    const float d2tdy2 = in[idx - width] - 2.0f * center + in[idx + width];
    out[idx] = center + 0.2f * (d2tdx2 + d2tdy2);
  } else {
    out[idx] = in[idx];
  }
}

void store(int step, int height, int width, const std::vector<float> &data) {
  std::ofstream file(std::string(output_dir) + "/heat_" +
                          std::to_string(step) + ".bin",
                      std::ios::binary);
  file.write(reinterpret_cast<const char *>(&height), sizeof(int));
  file.write(reinterpret_cast<const char *>(&width), sizeof(int));
  file.write(reinterpret_cast<const char *>(data.data()),
             static_cast<std::streamsize>(data.size() * sizeof(float)));
}

} // namespace big

int main() {
  using namespace big;

  const long long cells = static_cast<long long>(height) * width;
  const size_t bytes = static_cast<size_t>(cells) * sizeof(float);

  std::printf("Grid: %d x %d (%lld cells, %.2f MB per buffer)\n", height,
              width, cells, bytes / (1024.0 * 1024.0));
  std::printf("Frames: %d, steps per frame: %d, total simulation steps: %d\n",
              num_frames, steps_per_frame, num_frames * steps_per_frame);

  std::filesystem::create_directories(output_dir);

  float *d_prev = nullptr;
  float *d_next = nullptr;
  CUDA_CHECK(cudaMalloc(&d_prev, bytes));
  CUDA_CHECK(cudaMalloc(&d_next, bytes));

  const int threads = 256;
  const int blocks = static_cast<int>((cells + threads - 1) / threads);

  init_kernel<<<blocks, threads>>>(d_prev, height, width);
  CUDA_CHECK(cudaGetLastError());
  CUDA_CHECK(cudaDeviceSynchronize());

  std::vector<float> h_frame(static_cast<size_t>(cells));

  const auto t_start = std::chrono::high_resolution_clock::now();

  for (int write_step = 0; write_step < num_frames; write_step++) {
    for (int compute_step = 0; compute_step < steps_per_frame;
         compute_step++) {
      simulate_kernel<<<blocks, threads>>>(d_prev, d_next, height, width);
      CUDA_CHECK(cudaGetLastError());
      std::swap(d_prev, d_next);
    }
    CUDA_CHECK(cudaDeviceSynchronize());

    CUDA_CHECK(cudaMemcpy(h_frame.data(), d_prev, bytes,
                           cudaMemcpyDeviceToHost));
    store(write_step, height, width, h_frame);

    const auto now = std::chrono::high_resolution_clock::now();
    const double elapsed =
        std::chrono::duration<double>(now - t_start).count();
    std::printf("  wrote frame %d/%d (t = %.2f s)\n", write_step + 1,
                num_frames, elapsed);
  }

  const auto t_end = std::chrono::high_resolution_clock::now();
  const double total_seconds =
      std::chrono::duration<double>(t_end - t_start).count();
  std::printf("Total simulation + I/O time: %.2f s\n", total_seconds);

  CUDA_CHECK(cudaFree(d_prev));
  CUDA_CHECK(cudaFree(d_next));

  return 0;
}
