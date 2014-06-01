#include "suffix-array.h"
#include "sais.h"
#include <cassert>
#include <iostream>
#include <limits>

using std::string;
using Index = SuffixArray::Index;

SuffixArray::SuffixArray(const char* text, size_t len) {
  assert(len < std::numeric_limits<Index>::max());
  sa_.resize(len + 2);
  lcp_.resize(len + 2);
  // replace nulls with \x01 since sais doesn't support null characters
  text_.resize(len + 2, 0);
  for (Index i = 0; i < static_cast<Index>(len); ++i) {
    char c = text[i];
    if (c == 0) {
       c = 1;
    }
    text_[i] = c;
  }
  sais(reinterpret_cast<unsigned char*>(&text_[0]), &sa_[0], &lcp_[0], len + 1);
  sa_.pop_back();
  lcp_.pop_back();
}
SuffixArray::SuffixArray(const SuffixArray& o) :
  text_(o.text_), sa_(o.sa_), lcp_(o.lcp_) { }

SuffixArray::SuffixArray(SuffixArray&& o) :
  text_(std::move(o.text_)), sa_(std::move(o.sa_)), lcp_(std::move(o.lcp_)) { }

int SuffixArray::compare(const string& pattern, Index p) const {
  int d = 0;
  for (Index i = 0; i < static_cast<Index>(pattern.size()) && d == 0; ++i) {
    if (p + i >= static_cast<Index>(text_.size())) {
      d = -1;
      break;
    }
    unsigned char c = text_[p + i];
    unsigned char pc = pattern[i];
    if (c < pc) d = -1;
    else if (c > pc) d = 1;
  }
  return d;
}

// returns first index p with suffix[p] <= pattern
Index SuffixArray::left_rec(const string& pattern, Index left, Index right) const {
  if (left + 1 >= right) {
    return right;
  }
  Index m = (left + right) >> 1;
  Index p = sa_[m];
  int c = compare(pattern, p);
  if (c < 0) { // suffix < pattern
    return left_rec(pattern, m, right);
  } else {
    assert(c == 0);
    return left_rec(pattern, left, m);
  }
}

// returns first index p with suffix[p] > pattern
Index SuffixArray::right_rec(const string& pattern, Index left, Index right) const {
  if (left + 1 >= right) {
    return right;
  }
  int m = (left + right) >> 1;
  int p = sa_[m];
  int c = compare(pattern, p);
  if (c > 0) { // suffix >= pattern
    return right_rec(pattern, left, m);
  } else {
    assert(c == 0);
    return right_rec(pattern, m, right);
  }
}

SuffixArray::SuffixRange SuffixArray::locate_rec(const string& pattern,
                                                Index left,
                                                Index right) const {
  if (left == right) {
    return SuffixRange(left, right);
  }
  int m = (left + right) >> 1;
  int p = sa_[m];
  int c = compare(pattern, p);
  if (c != 0 && left + 1 == right) return SuffixRange(left, left);
  if (c == 0) {
    return SuffixRange(left_rec(pattern, left, m),
                         right_rec(pattern, m, right));
  } else if (c < 0) {
    return locate_rec(pattern, m, right);
  } else {
    return locate_rec(pattern, left, m);
  }
}
SuffixArray::SuffixRange SuffixArray::locate(const string& pattern) const {
  SuffixRange ret = locate_rec(pattern, 0, sa_.size());
  return ret;
}
