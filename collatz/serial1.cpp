#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <vector>

using namespace std;

int main(int argc, char* argv[]) {
  if (argc != 3) {
    cout << "argument (int) E (for experiment number) and (int) N is required!";
    return 0;
  }

  unsigned long long N = stoull(argv[2]);
  string experiment_number = argv[1];
  auto start = chrono::steady_clock::now();
  unordered_map<unsigned long long, unsigned long long> peaks;
  unordered_map<unsigned long long, unsigned long long> steps;
  unordered_map<unsigned long long, unsigned long long> stopping_numbers;
  unsigned long long stopping_times = 0;
  unsigned long long max_peak = 0;
  unsigned long long too_large = 0;

  for (unsigned long long i = 1; i < N; i++) {
    unsigned long long x = i;
    unsigned long long step = 0;
    unsigned long long cur_peak = x;
    bool large = false;
    while (x != 1) {
      cur_peak = max(cur_peak, x);
      max_peak = max(max_peak, x);
      if (x % 2 == 0) {
        x = x / 2;
      } else if (x > (numeric_limits<unsigned long long>::max() - 1) / 3) {
        too_large++;
        large = true;
        break;
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
  unsigned long long average_stopping_time = stopping_times / N;
  auto computation_done = chrono::steady_clock::now();
  auto elapsed = chrono::duration<double>(computation_done - start);

  ofstream summary("summary_" + experiment_number + ".txt");
  if (!summary) {
    cerr << "Could not open file\n";
    return 1;
  }

  summary << "Computation time: " << elapsed.count() << " seconds\n";
  summary << "N=" << N << '\n';
  summary << "failure_count=" << too_large << '\n';
  summary << "sum_stopping_times=" << stopping_times << '\n';
  summary << "average_stopping_time=" << average_stopping_time << '\n';
  summary << "maximum_peak=" << max_peak << '\n';
}