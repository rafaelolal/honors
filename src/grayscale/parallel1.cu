#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <thrust/execution_policy.h>
#include <thrust/iterator/counting_iterator.h>
#include <thrust/random.h>
#include <thrust/transform.h>
#include <thrust/universal_vector.h>
#include <vector>

using namespace std;

void grayscale(const vector<uint8_t[3]>& image, vector<uint8_t>& output) {
  for (int i = 0; i < image.size(); i++) {
    auto pixel = image[i];
    output[i] = 0.21 * pixel[0] + 0.72 * pixel[1] + 0.07 * pixel[2];
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cout
        << "argument (int) E (for experiment number) and (int) N is required!";
    return 0;
  }

  // Convert N to megapixels
  double N = stod(argv[2]) * 1'000'000;
  string experiment_number = argv[1];

  auto start = chrono::high_resolution_clock::now();

  // Generate random image
  int width = sqrt(N);
  int height = width;
  thrust::universal_vector<uint8_t> image(width * height * 3);

  thrust::transform(thrust::device, thrust::make_counting_iterator(0),
                    thrust::make_counting_iterator(width * height * 3),
                    image.data(), [] __device__(int x) {
                      thrust::default_random_engine rng(x);
                      thrust::uniform_int_distribution<int> color(0, 255);
                      return static_cast<uint8_t>(color(rng));
                    });

  auto gen_image_done = chrono::high_resolution_clock::now();

  chrono::duration<double, milli> elapsed = gen_image_done - start;
  std::cout << "Time to generate image: " << elapsed.count() << " ms\n";

  thrust::universal_vector<uint8_t> output(width * height);
  thrust::transform(thrust::device, thrust::make_counting_iterator(0),
                    thrust::make_counting_iterator(width * height),
                    output.data(),
                    [image_ptr = image.data()] __device__(int i) {
                      return static_cast<uint8_t>(0.21 * image_ptr[i * 3] +
                                                  0.72 * image_ptr[i * 3 + 1] +
                                                  0.07 * image_ptr[i * 3 + 2]);
                    });

  auto grayscale_done = chrono::high_resolution_clock::now();
  elapsed = grayscale_done - gen_image_done;
  std::cout << "Time to grayscale image: " << elapsed.count() << " ms\n";

  // Write output!
  ofstream file("image.bin", ios::binary);
  if (!file) {
    std::cout << "Could not open output file";
    return 1;
  }

  file.write(
      reinterpret_cast<const char*>(thrust::raw_pointer_cast(output.data())),
      static_cast<streamsize>(output.size()));

  auto writing_done = chrono::high_resolution_clock::now();
  elapsed = writing_done - grayscale_done;
  std::cout << "Time to write image: " << elapsed.count() << " ms\n";

  // Write experiment summary
  ofstream summary("summary_" + experiment_number + ".txt");

  if (!summary) {
    cerr << "Could not open file!";
    return 1;
  }

  summary << "Computation time: "
          << chrono::duration<double>(grayscale_done - gen_image_done).count()
          << " seconds\n";
  summary << "N=" << argv[2] << '\n';
  summary << "width=" << width << '\n';
  summary << "height=" << height << '\n';
  summary << "pixel_count=" << width * height << '\n';
  summary << "image_generation_time="
          << chrono::duration<double>(gen_image_done - start).count() << '\n';

  summary.close();

  return 0;
}