#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <functional>
#include <bitset>
#include <climits>
#include <utility>
#include <random>

#include "hashpack.h"
#include "simple-hashmap.h"
#include "util.h"
#include "timers.hpp"
#include "sep-chaining.h"
#include "rehashset.h"

using namespace std;

template <typename Key>
Key flip_bits(const Key k) {
    bitset<numeric_limits<Key>::digits> result(k);
    for (size_t i = 0; i < result.size()/2; ++i) {
        const bool temp = result[i];
        result[i] = result[result.size() - i];
        result[result.size() - i] = temp;
    }
    return result.to_ullong();
}

template <typename Table, bool FlipBits = false, typename Word>
size_t probe_length(size_t log_num_keys, const vector<Word>& to_insert) {
    Table ht(log_num_keys);
    size_t result = 0;
    for (const auto k : to_insert) {
        result += ht.Insert(FlipBits ? flip_bits(k) : k).second;
    }
    return result;
}

size_t dummy = 0;

template <typename Table, bool FlipBits = false, typename Word>
size_t build_time(size_t log_num_keys, const vector<Word>& to_insert) {
    Table ht(log_num_keys);
    const auto cycles_start = RDTSC_START();
    for (const auto k : to_insert) {
        dummy += ht.Insert(FlipBits ? flip_bits(k) : k).second;
    }
    const auto cycles_end = RDTSC_FINAL();
    return cycles_end - cycles_start;
}


int main() {
  vector<size_t> results;
  static const size_t LOG_NUM_KEYS = 20;
  vector<uint64_t> to_insert(1 << LOG_NUM_KEYS);
  iota(to_insert.begin(), to_insert.end(), get64rand());
  {
      std::random_device rd;
      std::mt19937 g(rd());
      shuffle(to_insert.begin(), to_insert.end(), g);
  }
  for (int i = 0; i < 1000; ++i) {
      results.push_back(build_time<
          // SplitHashSet<uint64_t, MultiplyShift64Pack, Zobrist64Pack, 4>,
          // HashSet<uint64_t, MultiplyShift64Pack>,
          ReHashSet<uint64_t, MultiplyShift64Pack>,
          // HashSet<uint64_t, Zobrist64Pack>,
          //          SepChain<uint64_t, MultiplyShift64Pack>,
          /* FlipBits */ false>(LOG_NUM_KEYS, to_insert));
  }
  cerr << dummy << endl;
  cout << "done" << endl;
  sort(results.begin(), results.end(), greater<uint64_t>());
  for (const auto v : results) {
    cout << v << endl;
  }
}
