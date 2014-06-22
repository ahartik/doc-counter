#pragma once
#include "suffix-array.h"
#include "wavelet/fast-bit-vector.h"
#include "wavelet/sparse-bit-vector.h"
#include "rmq.h"
#include <vector>

class SadaSparseCount {
 public:
  typedef SuffixArray::Index Index;
  typedef SuffixArray::SuffixRange SuffixRange;
  SadaSparseCount(const SuffixArray& sa) {
    using std::vector;
    vector<Index> ends;
    const unsigned char* text =
        reinterpret_cast<const unsigned char*>(sa.text());
    for (Index i = 0; i < sa.size(); ++i) {
      if (text[i] <= 1) {
        ends.push_back(i);
      }
    }
    SparseBitVector da(ends.begin(), ends.end());
    RMQSupport<Index> lcp_rmq(sa.lcp_data(), sa.size());

    vector<int> prev(ends.size() + 1, -1);
    vector<int> counts(sa.size(), 0);
    for (int i = 0; i < sa.size(); ++i) {
      int d = da.rank(sa.sa(i), 1);
      assert(d < lens.size() || i == 0);
      if (prev[d] == -1) {
        prev[d] = i;
      } else {
        int r = lcp_rmq.rmq_pos(prev[d]+1, i+1);
        counts[r]++;
        prev[d] = i;
      }
    }
    vector<int> count_pos;
    for (int i = 0; i < counts.size(); ++i) {
      if (counts[i] != 0) count_pos.push_back(i);
    }
    pos_ = SparseBitVector(count_pos.begin(),
                           count_pos.end());

    int c = 0;
    for (int i = 0; i < count_pos.size(); ++i) {
      c += counts[count_pos[i]];
      count_pos[i] = c;
    }
    count_ = SparseBitVector(count_pos.begin(),
                             count_pos.end());
  }

  size_t count(const SuffixRange& range, const std::string& pattern) const {
    if (range.first == range.second) return 0;
    size_t a = scount(range.first + 1);
    size_t b = scount(range.second);
    size_t dup = b - a;
    size_t len = range.second - range.first;
    return len - dup;
  }

  size_t count(const SuffixArray& sa, const std::string& pattern) const {
    SuffixArray::SuffixRange range = sa.locate(pattern);
    return count(range, pattern);
  }

  size_t byteSize() const {
    return (pos_.bitSize() + count_.bitSize()) / 8 + 1;
  }

 private:
  int scount(int i) const {
    // How many counts?
    int c = pos_.rank(i, 1);
    // What is the total?
    int r = count_.select(c, 1);
    // Fix end:
    return r;
  }

  SparseBitVector pos_;
  SparseBitVector count_;
};
