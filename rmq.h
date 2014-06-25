#pragma once
#include <vector>
#include <iostream>
#include <cstddef>
#include <climits>
#include <cmath>

template <typename T>
class RMQSupport {
 public:
  RMQSupport(const T* arr, size_t n) :
    array_(arr), size_(n) {
    depth_ = 1 + log2(n-1);
    int loglog = 1 + log2(depth_ - 1);
    int skip = 1 << loglog; // = 4
    tree_.resize(1<<(depth_ - loglog + 1));
    tree_[0] = -1;
    // build tree
    size_t base = 1 << (depth_ - loglog);
    
    for (size_t i = 0; i < n / skip; ++i) {
      int x = i * skip;
      if (x >= n) {
        tree_[base + i] = -1;
        continue;
      }
      for (size_t j = i * skip; j < (i+1)*skip; ++j) {
        if (j >= n) break;
        if (arr[j] < arr[x]) {
          x = j;
        }
      }
      tree_[base + i] = x;
    }

    for (size_t i = base-1; i > 0; --i) {
      tree_[i] = std::min(tree_[i * 2], tree_[i * 2 + 1]);
      if (arr[tree_[i * 2]] < arr[tree_[i * 2 + 1]]) {
        tree_[i] = tree_[i * 2];
      } else {
        tree_[i] = tree_[i * 2 + 1];
      }
    }
    // debug
#if 0
    for (size_t i = 1; i < tree_.size(); ++i) {
      if ((i & -i) == i) std::cout << "\n";
      std::cout << tree_[i] << " ";
    }
    std::cout << "\n";
    for (size_t i = 0; i < n; ++i)
      std::cout << arr[i] << " ";
    std::cout << "\n";
#endif
  }
  size_t rmq(size_t begin, size_t end) {
    return array_[rmq_pos(begin, end)];
  }
  size_t rmq_pos(size_t begin, size_t end) {
    return rmq_rec(1, begin, end, 0, 1 << depth_);
  }
 private:
  size_t rmq_rec(size_t n, size_t begin, size_t end, size_t l, size_t r) {
    if (end <= l || r <= begin) {
      return size_;
    }
#if 1
    if (n >= tree_.size()) {
      l = std::max(l, begin);
      r = std::min(r, end);
      size_t m = l;
      for (size_t i = l+1; i < r; ++i) {
        if (array_[i] < array_[m])
          m = i;
      }
      return m;
    }
#else 
    if (n >= tree_.size()) {
      return n - tree_.size();
    }
#endif
    if (begin <= l && r <= end) {
      return tree_[n];
    }
    size_t ml = rmq_rec(n * 2, begin, end, l, (l + r) / 2);
    size_t mr = rmq_rec(n * 2 + 1, begin, end, (l + r) / 2, r);
    if (ml == size_) return mr;
    if (mr == size_) return ml;
    if (array_[ml] < array_[mr]) return ml;
    return mr;
  }
  size_t parent(size_t x) {
    return x / 2;
  }
  size_t child(size_t x) {
    return x * 2;
  }
  int depth_;
  std::vector<int> tree_;
  const T* array_;
  size_t size_;
};
