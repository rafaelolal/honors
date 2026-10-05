#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
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
    cout << "argument (int) E (for experiment number) and (int) N is required!";
    return 0;
  }

  // Convert N to megapixels
  double N = stod(argv[2]) * 1'000'000;
  string experiment_number = argv[1];

  auto start = chrono::high_resolution_clock::now();

  // Generate random image
  int width = sqrt(N);
  int height = width;
  mt19937 rng(random_device{}());
  uniform_int_distribution<int> color(0, 255);
  vector<uint8_t[3]> image(width * height);
  for (int i = 0; i < width * height; i++) {
    image[i][0] = static_cast<uint8_t>(color(rng));
    image[i][1] = static_cast<uint8_t>(color(rng));
    image[i][2] = static_cast<uint8_t>(color(rng));
  }

  auto gen_image_done = chrono::high_resolution_clock::now();

  chrono::duration<double, milli> elapsed = gen_image_done - start;
  cout << "Time to generate image: " << elapsed.count() << " ms\n";

  vector<uint8_t> output(width * height);
  grayscale(image, output);

  auto grayscale_done = chrono::high_resolution_clock::now();
  elapsed = grayscale_done - gen_image_done;
  cout << "Time to grayscale image: " << elapsed.count() << " ms\n";

  // Write output!
  ofstream file("image.bin", ios::binary);
  if (!file) {
    cout << "Could not open output file";
    return 1;
  }

  auto writing_done = chrono::high_resolution_clock::now();
  elapsed = writing_done - grayscale_done;
  cout << "Time to write image: " << elapsed.count() << " ms\n";

  file.write(reinterpret_cast<const char*>(output.data()),
             static_cast<streamsize>(output.size()));

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