#include "rmq.h"
#include <cassert>
#include <cstdlib>
#include <algorithm>

const int arrsize = 1000;
int arr[arrsize];

template<typename RMQT>
void test(RMQT& rmq) {
  srand(0);
  size_t count = 100;
  size_t checksum = 0;
  for (int i = 0; i < count; ++i) {
    int l = rand() % arrsize;
    int r = l + 1 + rand() % (arrsize - l);
    int mp = rmq.rmq(l,r);
    checksum += mp + checksum << 1;
    int real_min = std::min_element(arr + l, arr + r) - arr;
    if (mp != real_min) {
      std::cout << "From " << l << " to " << r << "\n";
      std::cout << "Error: " << "rmq = " << arr[mp] << " real = " << arr[real_min] << "\n";
    }
  }
  std::cout << "CHECK: " << checksum << "\n";
}

// int arrsize = sizeof(arr) / sizeof(int);
int main() {
  for (int i = 0; i < arrsize; ++i)
    arr[i] = rand()%100;

  RMQ<int> rmq(arr, arrsize);
  std::cout << "RMQ<int>\n";
  test(rmq);
  FastRMQ<int> frmq(arr, arrsize);
  std::cout << "FastRMQ<int>\n";
  test(frmq);
}
