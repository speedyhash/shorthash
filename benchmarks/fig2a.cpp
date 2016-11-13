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

template <typename Table, bool FlipBits = false, typename Word>
vector<size_t> build_times(size_t samples, size_t log_num_keys,
                           const vector<Word>& to_insert) {
    vector<size_t> result(samples);
    ::std::generate(result.begin(), result.end(), [&]() {
        return build_time<Table, FlipBits>(log_num_keys, to_insert);
    });
    sort(result.begin(), result.end(), ::std::greater<uint64_t>());
    return result;
}

template <typename... Tables>
void print_build_times(size_t samples, size_t log_num_keys,
                       const vector<uint64_t>& to_insert) {
    const array<vector<size_t>, sizeof...(Tables)> results(
        {build_times<Tables>(samples, log_num_keys, to_insert)...});
    const array<string, sizeof...(Tables)> names({Tables::Name()...});
    for (const auto& name : names) {
        cout << name << ' ';
    }
    cout << endl;
    for (size_t i = 0; i < results[0].size(); ++i) {
        for (const auto& result : results) {
            cout << result[i] << ' ';
        }
        cout << endl;
    }
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
  print_build_times<HashSet<uint64_t, MultiplyShift64Pack>,
                    HashSet<uint64_t, Zobrist64Pack>,
                    ReHashSet<uint64_t, MultiplyShift64Pack>>(
      1 << 12, LOG_NUM_KEYS, to_insert);
}
