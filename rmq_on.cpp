#pragma once

#include "wavelet/int-array.h"
#include "wavelet/fast-bit-vector.h"

#include "small-rmq.h"

#include <cmath>
#include <chrono>
#include <vector>
#include <cstddef>
#include <memory>
#include <functional>
#include <algorithm>

#include <iostream>

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

// Solution in <O(n log n), O(1)>
template<typename T, typename Func = ArrWrap<T>>
class FastRMQ {
 public:
  FastRMQ(Func arr, size_t n) : arr_(arr) {
    m.resize(1 + log2(n-1));
    for (size_t w = 0; w < m.size(); ++w) {
      m[w].resize(n);
      m[w][0] = 0;
    }
    for (size_t i = 1; i < n; ++i) {
      if (arr(i) < arr(i-1)) {
        m[0][i] = i;
      } else {
        m[0][i] = i - 1;
      }
    }
    for (size_t w = 1; w < m.size(); ++w) {
      int span = 1 << w;
      for (size_t i = 0; i < n; ++i) {
        int a = m[w-1][i];
        int b = m[w-1][std::max<int>(i - span, 0)];
        if (arr(a) < arr(b)) {
          m[w][i] = a;
        } else {
          m[w][i] = b;
        }
      }
    }
  }
  size_t rmq(size_t begin, size_t end) const {
    if (begin + 1 >= end)return begin;
    if (begin + 2 == end) {
      return m[0][end-1];
    }
    int w = log2(end - begin - 1);
    assert(begin + (1<<w) < end);
    assert(begin + (2<<w) >= end);

    int a = m[w-1][end - 1];
    int b = m[w-1][begin + (1 << w) - 1];

    if (arr_(a) < arr_(b)) return a;
    else return b;
  }
 private:
  Func arr_;
  std::vector<std::vector<int>> m;
};

template<typename T, typename Func = ArrWrap<T>>
class RMQ {
 private:
  typedef std::function<const T&(size_t i)> FuncWrap;
 public:
  RMQ(Func arr, size_t n, int depth = 1) 
      : arr_(arr),
        size_(n),
        small_rmq_(1)
  {
    bs_ = (1 + log2(n-1)) / 4;
    size_t num_blocks = 1 + (n-1) / bs_;
    small_rmq_ = SmallRMQ(bs_);
    std::cout << "bs = " << bs_ << "\n";
    std::cout << "num_blocks = " << num_blocks << "\n";
    if (bs_ <= 1 || num_blocks <= 1) {
      bs_ = 0;
    } else {
      block_min_ = IntArray(1 + log2(bs_ - 1), num_blocks);
      block_id_ = IntArray(2 * bs_, num_blocks);
      for (size_t i = 0; i < num_blocks; ++i) {
        size_t m = 0;
        size_t start = i * bs_;
        for (size_t j = 1; j < bs_; ++j) {
          if (arr_(start + j) < arr_(start + m)) {
            m = j;
          }
        }
        block_min_.set(i, m);
        uint64_t block_id = small_rmq_.addRMQ<Func>(arr, start);
        block_id_.set(i, block_id);
      } 
      using namespace std::placeholders;
      // FuncWrap block_func =
      //     std::bind(&RMQ<T,Func>::GetBlockMin, this, _1);
      BlockFunc block_func {this};
      // block_rmq_.reset(new FastRMQ<T, FuncWrap>(block_func, num_blocks));
      block_rmq_.reset(new FastRMQ<T, BlockFunc>(block_func, num_blocks));
    }
  }
  size_t rmq(size_t begin, size_t end) const {
    if (bs_ == 0) return brute_rmq(begin, end);
    if (begin == end) return end;
    size_t start_block = 1 + (begin / bs_);
    size_t end_block = end / bs_;

    size_t ret = begin;
    if (start_block > end_block) {
      return brute_rmq(begin, end);
    }
    if (start_block < end_block) {
      size_t min_block = block_rmq_->rmq(start_block, end_block);
      size_t middle_min = min_block * bs_ + block_min_.get(min_block);
      ret = middle_min;
    }
    if (begin != start_block * bs_) {
      size_t start_min = brute_rmq(begin, start_block * bs_);
      if (arr_(start_min) < arr_(ret))
        ret = start_min;
    }
    if (end != end_block * bs_) {
      size_t end_min = brute_rmq(end_block * bs_, end);
      if (arr_(end_min) < arr_(ret))
        ret = end_min;
    }
    assert(begin <= ret && ret < end);
    return ret;
  }
 private:
  size_t brute_rmq(size_t begin, size_t end) const {
    if (begin == end) return end;
    if (bs_ != 0) {
      size_t block = begin / bs_;
      begin -= block * bs_;
      end -= block * bs_;
      assert(begin >= 0 && end <= bs_);
      return block * bs_ + small_rmq_.rmq(block_id_.get(block), begin, end);
    }
    size_t r = begin;
    for (size_t i = begin + 1; i < end; ++i) {
      if (arr_(i) < arr_(r)) r = i;
    }
    return r;
  }

  const T& blockMin(size_t i) const {
    size_t block_start = bs_ * (i / bs_);
    return arr_(block_start + block_min_.get(i));
  }

  struct BlockFunc {
    const RMQ<T, Func>* rmq;
    const T& operator()(size_t i) const {
      return rmq->blockMin(i);
    }
  };

  IntArray block_min_;
  Func arr_;
  // std::unique_ptr<RMQ<T, FuncWrap>> block_rmq_;
  std::unique_ptr<FastRMQ<T, BlockFunc>> block_rmq_;

  SmallRMQ small_rmq_;
  IntArray block_id_;

  size_t size_;
  size_t bs_;
};

const int arrsize = 1000 * 1000 * 100;
int arr[arrsize];

int main() {
  for (int i = 0; i < arrsize; ++i)
    arr[i] = i;

  using namespace std::chrono;
  std::chrono::high_resolution_clock clock;
  auto cons_start = clock.now();
  std::cout << "construction: " << std::flush;
  RMQ<int> rmq(arr, arrsize);
  FastRMQ<int> fast_rmq(arr, arrsize);
  auto cons_end = clock.now();
  std::cout << duration_cast<milliseconds>(cons_end-cons_start).count() << "ms\n";
  size_t count = 10000;
  size_t checksum = 0;
  for (int i = 0; i < count; ++i) {
    int l = rand() % arrsize;
    int r = l + 1 + rand() % (arrsize - l);
    int mp = rmq.rmq(l,r);
    checksum += mp + checksum << 1;
//     int real_min = std::min_element(arr + l, arr + r) - arr;
//     int fast_min = fast_rmq.rmq(l,r);
//     if (arr[mp] != arr[fast_min]) {
//       std::cout << "From " << l << " to " << r << "\n";
//       std::cout << "Error: " << "rmq = " << arr[mp] << " fast = " << arr[fast_min] << "\n";
//       std::cout << "Error: " << "rmq_pos = " << mp << "\n";
// }
  }
  auto rmq_end = clock.now();
  std::cout << duration_cast<nanoseconds>(rmq_end-cons_end).count() / count << "ns/rmq\n";
  std::cout << "CHECK: " << checksum << "\n";
}
