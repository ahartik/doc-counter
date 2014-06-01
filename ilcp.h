#pragma once
#include "suffix-array.h"

template<typename Wavelet>
class ILCP {
 public:
  typedef SuffixArray::Index Index;
  typedef SuffixArray::SuffixRange SuffixRange;
  ILCP(const SuffixArray& sa) {
    Index length = sa.size();
    std::vector<Index> ilcp(length + 2);
    std::vector<Index> text_lcp(length+1);
    Index start = 0;
    int num_docs = 0;
    const char* text = sa.text();
    for (Index i = 0; i <= static_cast<Index>(length); ++i) {
      if (text[i] <= 1) {
        const char* doc = text + start;
        Index doc_len = i - start;
        SuffixArray doc_sa(doc, doc_len);
        for (Index j = 0; j <= doc_len; ++j) {
          Index p = doc_sa.sa(j);
          Index lcp = doc_sa.lcp(j);
          text_lcp[start + p] = lcp;
        }
        num_docs++;
        start = i;
      }
    }
    for (Index i = 0; i <= length; ++i) {
      ilcp[i] = text_lcp[sa.sa(i)];
    }

#if 0
    for (size_t i = 0; i <= length; ++i) {
      printf("%3zi", i);
    }
    printf("\n");
    for (size_t i = 0; i <= length; ++i) {
      printf("%3i", ilcp[i]);
    }
    printf("\n");
    for (size_t i = 0; i <= length; ++i) {
      char c = text[sa_.sa(i)];
      if (isprint(c)) printf("  %c", c);
      else printf("   ");
    }
    printf("\nTEXT:\n");
    for (size_t i = 0; i <= length; ++i) {
      printf("%3i", text_lcp[i]);
    }
    printf("\n");
    for (size_t i = 0; i <= length; ++i) {
      char c = text[i];
      if (isprint(c)) printf("  %c", c);
      else printf("   ");
    }
    printf("\n");
#endif
    text_lcp = std::vector<Index>();
    wt_ = Wavelet(ilcp.begin(), ilcp.end());
  }

  size_t count(const SuffixRange& range, const std::string& pattern) const {
    return wt_.rankLE(range.second, pattern.size() - 1) -
           wt_.rankLE(range.first, pattern.size() - 1);
  }

  size_t count(const SuffixArray& sa, const std::string& pattern) const {
    SuffixArray::SuffixRange range = sa.locate(pattern);
    return count(range, pattern);
  }

  size_t byteSize() const {
    return wt_.bitSize() / 8 + 1;
  }

 private:
  Wavelet wt_;
};
