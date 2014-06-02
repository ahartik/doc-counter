#include <gtest/gtest.h>
#include <iostream>

#include "wavelet/rrr-bit-vector.h"
#include "wavelet/rle-wavelet.h"
#include "ilcp.h"
#include "brute.h"

template<typename T>
class CountTest : public ::testing::Test {

};

typedef ::testing::Types<
  BalancedWavelet<>,
  SkewedWavelet<>,
  RLEWavelet<BalancedWavelet<>>,
  RLEWavelet<SkewedWavelet<>>,
  BalancedWavelet<RRRBitVector>,
  SkewedWavelet<RRRBitVector>,
  RLEWavelet<BalancedWavelet<RRRBitVector>>,
  RLEWavelet<SkewedWavelet<RRRBitVector>>
  > WaveletTypes;

TYPED_TEST_CASE(CountTest, WaveletTypes );

TYPED_TEST(CountTest, Simple) {
  std::string text = "antero\001ananas\001banana";
  SuffixArray sa(text.c_str(), text.size());
  ILCP<TypeParam> ilcp(sa);
  EXPECT_EQ(ilcp.count(sa, "an"), 3);
  EXPECT_EQ(ilcp.count(sa, "b"), 1);
  EXPECT_EQ(ilcp.count(sa, "tero"), 1);
  EXPECT_EQ(ilcp.count(sa, "ana"), 2);
}

TEST(CountTest, Brute) {
  std::string text = "antero\001ananas\001banana";
  SuffixArray sa(text.c_str(), text.size());
  BruteCount c(sa);
  EXPECT_EQ(c.count(sa, "an"), 3);
  EXPECT_EQ(c.count(sa, "b"), 1);
  EXPECT_EQ(c.count(sa, "tero"), 1);
  EXPECT_EQ(c.count(sa, "ana"), 2);
}

TYPED_TEST(CountTest, CompareToBrute) {
  std::string text = 
      "ananas\001"
      "amsterdam\001"
      "apple\001"
      "banana\001"
      "cantelope\001"
      "cat\001"
      "dog\001"
      "dublin\001";
  SuffixArray sa(text.c_str(), text.size());
  ILCP<TypeParam> ilcp(sa);
  BruteCount brute(sa);
  // test for each short substring
  for (int len = 1; len <= 4; ++ len) {
    for (size_t i = 0; i < text.size(); ++i) {
      std::string pattern = text.substr(i, len);
      if (pattern.find("\001") != std::string::npos) continue;
      EXPECT_EQ(brute.count(sa, pattern),
                ilcp.count(sa, pattern))
          << "pattern = " << pattern << "\n";
    }
  }
}
