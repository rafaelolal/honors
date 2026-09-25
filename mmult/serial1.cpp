#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

using namespace std;

int main(int argc, char* argv[]) {
  if (argc != 3) {
    cout << "argument (int) E (for experiment number) and (int) N is required!";
    return 0;
  }

  int N = stoi(argv[2]);
  string experiment_number = argv[1];

  vector<int> A(N * N);
  vector<int> B(N * N);
  vector<int> C(N * N);

  /////////////////////////////////////////
  //fill in the matrices with random numbers
  mt19937 gen;
  uniform_int_distribution<int> distr(numeric_limits<int>::min(),
                                      numeric_limits<int>::max());

  auto start = chrono::high_resolution_clock::now();
  for (int i = 0; i < N * N; i++) {
    A[i] = distr(gen);
    B[i] = distr(gen);
  }
  auto gen_done = chrono::high_resolution_clock::now();
  auto time_to_gen = chrono::duration<double>(gen_done - start).count();

  cout << "time_to_gen " << time_to_gen << "\n";

  
  ///////////////////////////////////
  // compute A x B
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      int c = 0;
      for (int k = 0; k < N; k++) {
        c += A[i * N + k] * B[k * N + j];
      }
      C[i * N + j] = c;
    }
  }
  auto mult_done = chrono::high_resolution_clock::now();
  auto time_to_mult = chrono::duration<double>(mult_done - gen_done).count();

  cout << "time_to_mult " << time_to_mult << "\n";


  ////////////////////////////////////////
  //print stats 
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
