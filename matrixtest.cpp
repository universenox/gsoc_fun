#include "matrix.hpp"
#include <iostream>
#include <chrono>
#include <numeric>
#include <cstdlib>
#include <algorithm>

using namespace std;

int main()
{
  // using two size x size matrices for testing
  const int SIZE = 20;
  int n = SIZE;
  
  vector<int> va (SIZE * SIZE);
  vector<int> vb (SIZE * SIZE);

  // uses lambda function to generate each elem. of the matrix
  generate(va.begin(), va.end(), [&n, SIZE]() { return n = n - 3; });
  generate(vb.begin(), vb.end(), [&n, SIZE]() { return n = n + 11; });

  matrix<int> a (SIZE, SIZE, va);
  matrix<int> b (SIZE, SIZE, vb);

  matrix<int> c;
  int scalar = 123;

  cout << "A\n" << a
       << "B\n" << b;
  
  cout << "adding A and B\n";
  auto start = std::chrono::system_clock::now();
  c = a + b;
  // std::cout << a + b
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> time_taken = end - start;
  cout << "time taken: " << time_taken.count() << " seconds\n";

  
  cout << "\nsubtracting B from A\n";
  start = std::chrono::system_clock::now();
  c = a - b;
  // std::cout << a - b;
  end = std::chrono::system_clock::now();
  time_taken = end - start;
  cout << "time taken: " << time_taken.count() << " seconds\n";

  
  cout << "\nmultiplying A by scalar\n";
  start = std::chrono::system_clock::now();
  c = a * scalar;
  // std::cout << a * 123;
  end = std::chrono::system_clock::now();
  time_taken = end - start;
  cout << "time taken: " << time_taken.count() << " seconds\n";

  
  cout << "\nmultiplying A and B\n";
  start = std::chrono::system_clock::now();
  c = a * b;
  // std::cout << a * b;
  end = std::chrono::system_clock::now();
  time_taken = end - start;
  cout << "time taken: " << time_taken.count() << " seconds\n";
  

  return 0;
}
