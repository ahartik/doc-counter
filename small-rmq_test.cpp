#include "small-rmq.h"
#include <iostream>
const int arrsize = 10;

SmallRMQ rmq(arrsize);

template<typename T>
struct ArrWrap {
  ArrWrap(const T* arr) {
    arr_ = arr;
  }
  const T& operator()(size_t i) const {
    return arr_[i];
  }
  const T* arr_;
};

void test(int* arr,int begin) {
  uint64_t id = rmq.addRMQ<ArrWrap<int>>(ArrWrap<int>(arr), begin);
  arr += begin;
  int count = 10;
  for (int i = 0; i < count; ++i) {
    int l = rand() % arrsize;
    int r = l + 1 + rand() % (arrsize - l);
    int mp = rmq.rmq(id, l, r);
    int real_min = std::min_element(arr + l, arr + r) - arr;
    if (arr[mp] != arr[real_min]) {
      std::cout << "From " << l << " to " << r << "\n";
      std::cout << "Error: " << "rmq = " << arr[mp] << " real = " << arr[real_min] << "\n";
      assert(false);
    }
  }
}


int main() {
  const size_t big_size = 1000000;
  int big[big_size];
  for (int& x : big) {
    x = rand() % 1000;
  }

  for (size_t i = 0; i < big_size; ++i) {
    test(big, i);
    test(big + i, 0);
  }
}
