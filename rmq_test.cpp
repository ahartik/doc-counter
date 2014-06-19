#include "rmq.h"
#include <cassert>
#include <cstdlib>
#include <algorithm>


const int arrsize = 100;
int arr[arrsize];
// int arrsize = sizeof(arr) / sizeof(int);
int main() {
  for (int i = 0; i < arrsize; ++i)
    arr[i] = rand()%100;
  RMQSupport<int> rmq(arr, arrsize);
  for (int i = 0; i < 100; ++i) {
    int l = rand() % arrsize;
    int r = l + 1 + rand() % (arrsize - l);
    int mp = rmq.rmq_pos(l,r);
    int real_min = std::min_element(arr + l, arr + r) - arr;
    if (mp != real_min && arr[mp] != arr[real_min]) {
      std::cout << "From " << l << " to " << r << "\n";
      std::cout << "Error: " << "rmq = " << arr[mp] << " real = " << arr[real_min] << "\n";
    }
  }
}
