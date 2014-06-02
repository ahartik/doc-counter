// Use sdsl for the suffix array
//
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <gflags/gflags.h>

#include "wavelet/rle-wavelet.h"
#include "wavelet/rrr-bit-vector.h"

#include "ilcp.h"
#include "brute.h"

DEFINE_string(structures, "balanced",
              "Comma-separated list of tested structures. Available structures"
              ": brute,balanced,skewed,rle,rle_skewed,balanced_rrr,skewed_rrr,"
              "rle_rrr,rle_skewed_rrr"
              );

DEFINE_string(pattern_file, "",
              "File containing patterns to be counted, each on its own line.");

DEFINE_string(document_file, "",
              "File containing all the documents null-separated.");
DEFINE_bool(print_counts, false, "Print counts to standard output. For debugging purposes.");

bool readBinaryFile(const std::string& filename, std::vector<char> *contents) {
  std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
  if (in)
  {
    in.seekg(0, std::ios::end);
    contents->resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&(*contents)[0], contents->size());
    in.close();
    return true;
  }
  return false;
}


template<typename Counter> 
void countPatterns(const SuffixArray& sa,
                   const std::vector<std::string>& patterns) {
  using namespace std::chrono;
  std::vector<SuffixArray::SuffixRange> ranges;
  ranges.reserve(patterns.size());
  for (const std::string& p : patterns) {
    ranges.push_back(sa.locate(p));
  }
  Counter counter(sa);
  uint64_t checksum = 0;
  std::chrono::high_resolution_clock clock;
  auto start = clock.now();

  for (size_t i = 0; i < patterns.size(); ++i) {
    const std::string& p = patterns[i];
    int c = counter.count(ranges[i], p);
    if (FLAGS_print_counts) {
      std::cout <<"\t" <<  p << ": " << c << "\n";
    }
    checksum = checksum * 31 + c;
  }
  auto end = clock.now();
  std::cout << duration_cast<nanoseconds>(end-start).count()/patterns.size()<< "ns/count\n";

  std::cout << "size: " << counter.byteSize() << "\n";
  std::cout << "checksum: " << checksum << "\n";
}

std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

int main(int argc, char** argv) {
  gflags::SetUsageMessage(
    "usage: ./doc_counter --structures struct1,struct2 --pattern_file pattern "
    " --document_file documents"
      );
  gflags::ParseCommandLineFlags(&argc, &argv, false);
  bool success = true;
  if (FLAGS_pattern_file.empty()) {
    std::cerr << "--pattern_file must be defined\n";
    success = false;
  }
  if (FLAGS_document_file.empty()) {
    std::cerr << "--document_file must be defined\n";
    success = false;
  }
  if (FLAGS_structures.empty()) {
    std::cerr << "--structures must be defined\n";
    success = false;
  }
  if (!success) {
    std::cerr << gflags::ProgramUsage() << std::endl;
    return 1;
  }
  // Read text
  std::vector<char> text;
  readBinaryFile(FLAGS_document_file, &text);
  SuffixArray sa(text.data(), text.size());
  // free memory.
  text = std::vector<char>(); 

  // Read patterns
  std::vector<std::string> patterns;
  std::ifstream pattern_in(FLAGS_pattern_file.c_str());
  for (std::string pattern; std::getline(pattern_in, pattern); ) {
    patterns.push_back(pattern);
  }

  std::unordered_map<std::string,
      std::function<void(const SuffixArray&, const std::vector<std::string>&)>>
    structFuncs;

  structFuncs["brute"] = &countPatterns<BruteCount>;
  structFuncs["skewed"] = &countPatterns<ILCP<
      SkewedWavelet<>>>;
  structFuncs["balanced"] = &countPatterns<ILCP<
      BalancedWavelet<>>>;
  structFuncs["rle"] = &countPatterns<ILCP<
      RLEWavelet<BalancedWavelet<>>>>;
  structFuncs["rle_skewed"] = &countPatterns<ILCP<
      RLEWavelet<SkewedWavelet<>>>>;
  structFuncs["skewed_rrr"] = &countPatterns<ILCP<
      SkewedWavelet<RRRBitVector>>>;
  structFuncs["balanced_rrr"] = &countPatterns<ILCP<
      BalancedWavelet<RRRBitVector>>>;
  structFuncs["rle_rrr"] = &countPatterns<ILCP<
      RLEWavelet<BalancedWavelet<RRRBitVector>>>>;
  structFuncs["rle_skewed_rrr"] = &countPatterns<ILCP<
      RLEWavelet<SkewedWavelet<RRRBitVector>>>>;
  
  for (const std::string& s : split(FLAGS_structures, ',')) {
    if (structFuncs.count(s)) {
      std::cout << "structure: " << s << "\n";
      structFuncs[s](sa, patterns);
    } else {
      std::cout << "unknown structure: " << s << "\n";
      return 1;
    }
    std::cout << "\n";
  }
}
