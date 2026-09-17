#include <fstream>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <vector>

using namespace std;

int main() {
  int N;
  cin >> N;
  unordered_map<int, int> peaks;
  unordered_map<int, int> steps;
  unordered_map<int, int> stopping_numbers;
  int stopping_times = 0;
  long long max_peak = 0;
  int too_large = 0;

  for (int i = 1; i < N; i++) {
    long long x = i;
    int step = 0;
    long long cur_peak = x;
    int large = 0;
    while (x != 1) {
      if (x > numeric_limits<int>::max()) {
        too_large++;
        large = 1;
        break;
      }
      cur_peak = max(cur_peak, x);
      max_peak = max(max_peak, x);
      if (x % 2 == 0) {
        x = x / 2;
      } else {
        x = 3 * x + 1;
      }
      step++;
    }
    if (large) {
      continue;
    }
    peaks[i] = cur_peak;
    steps[i] = step;
    stopping_times += step;
    stopping_numbers[step]++;
  }
  int average_stopping_time = stopping_times / N;

  ofstream stopping_histogram("stopping_histogram.txt");
  if (!stopping_histogram) {
    cerr << "Could not open file\n";
    return 1;
  }

  for (const auto& [key, value] : stopping_numbers) {
    stopping_histogram << key << "\t" << value << "\n";
  }

  ofstream results("results.txt");
  if (!results) {
    cerr << "Could not open file\n";
    return 1;
  }

  printf("rafael %d\n", peaks[4]);
  fflush(stdout);
  results << "Max peak: " << max_peak << "\n";
  results << "Stopping sum: " << stopping_times << "\n";
  results << "Average stopping: " << average_stopping_time << "\n";
  results << "Too large: " << too_large << "\n";
  for (int i = 1; i < N; i++) {
    results << i << "\t" << peaks[i] << "\t" << steps[i] << "\n";
  }
}