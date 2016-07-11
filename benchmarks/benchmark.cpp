#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

#include "hashpack.h"
#include "timers.hpp"

using namespace std;

static __attribute__((noinline)) uint64_t rdtsc_overhead_func(uint64_t dummy) {
    return dummy;
}

uint64_t global_rdtsc_overhead = (uint64_t)UINT64_MAX;

void RDTSC_SET_OVERHEAD(int repeat) {
    uint64_t cycles_start, cycles_final, cycles_diff;
    uint64_t min_diff = UINT64_MAX;
    for (int i = 0; i < repeat; i++) {
        __asm volatile("" ::: /* pretend to clobber */ "memory");
        cycles_start = RDTSC_START();
        rdtsc_overhead_func(1);
        cycles_final = RDTSC_FINAL();
        cycles_diff = (cycles_final - cycles_start);
        if (cycles_diff < min_diff)
            min_diff = cycles_diff;
    }
    global_rdtsc_overhead = min_diff;
    printf("rdtsc_overhead set to %d\n", (int)global_rdtsc_overhead);
}


typedef struct timing_stat_s {
  double min_opc;// minimal number of operations per cycle
  double max_opc;// maximal number of operations per cycle
  double avg_opc; // average number of operations per cycle
  bool wrong_answer;
} timing_stat_t;


/*
 * Prints the best number of operations per cycle where
 * test is the function call,  repeat is the number of times we should repeat
 * and size is the
 * number of operations represented by test.
 */
template <typename T>
inline timing_stat_t BEST_TIME(const T &hasher, int repeat, size_t size) {
    uint64_t cycles_start, cycles_final, cycles_diff;
    uint64_t min_diff = (uint64_t)-1;
    uint64_t max_diff = 0;
    uint64_t sum_diff = 0;
    bool wrong_answer = false;
    for (int i = 0; i < repeat; i++) {
        __asm volatile("" ::: /* pretend to clobber */ "memory");
        hasher.Restart();
        cycles_start = RDTSC_START();
        if (hasher.Hash() != hasher.expected_)
            wrong_answer = true;
        cycles_final = RDTSC_FINAL();
        cycles_diff = (cycles_final - cycles_start) - global_rdtsc_overhead;
        if (cycles_diff < min_diff)
            min_diff = cycles_diff;
        if (cycles_diff > max_diff)
            max_diff = cycles_diff;
        sum_diff += cycles_diff;
    }
    timing_stat_t stat;
    stat.wrong_answer =  wrong_answer;
    stat.min_opc = min_diff / (double) size;
    stat.max_opc = max_diff / (double)size;
    stat.avg_opc = sum_diff / (double) ( repeat * size );
    return stat;
}

void cache_flush(const void *b, size_t length) {
    if (nullptr == b) return;
    char *B = (char *)b;
    for (uint32_t k = 0; k < length; k += 64) {
        __builtin_ia32_clflush(B + k);
    }
}

template <typename HashDataType, typename HashValueType,
          HashValueType HashFunction(HashValueType, const HashDataType *)>
struct HashBench {
  inline void Restart() const {
    cache_flush(k_, sizeof(*k_));
  }

  inline HashValueType Hash() const {
    HashValueType sum = 0;
    for (uint32_t x = 0; x < length_; ++x) {
        sum += HashFunction(array_[x], k_);
    }
    return sum;
  }

  HashBench(const HashValueType *const array, const uint32_t length,
            const HashDataType *const k)
    : array_(array), length_(length), k_(k) {
      expected_ = Hash();
  }

  const HashValueType * const array_;
  const uint32_t length_;
  const HashDataType * const k_;
  HashValueType expected_;
};

static const int FIRST_FIELD_WIDTH = 20;
static const int FIELD_WIDTH = 10;

static inline uint32_t intlog(uint32_t x) {
  if(x == 0) return 32;
  return 32 - __builtin_clzl(x);
}


template <typename T>
inline timing_stat_t Bench(const typename T::Word *input, uint32_t length, int repeat) {
    typename T::Randomness * randomness = new typename T::Randomness();
    if(randomness == NULL) std::cerr << "Failure to allocate" << std::endl;
    T::InitRandomness(randomness);

    repeat = std::max(UINT32_C(1),repeat / intlog(length));
    HashBench<typename T::Randomness, typename T::Word, &T::HashFunction> demo(
        input, length, randomness);
    timing_stat_t answer =  BEST_TIME(demo, repeat, length);
    delete randomness;
    return answer;
}

struct NameWorker {
    template <typename Pack>
    static inline void Go() {
        cout << setw(FIELD_WIDTH)
             << string(Pack::NAME).substr(0, FIELD_WIDTH - 1);
    }
    static inline void Stop() { cout << endl; }
};

struct SizeWorker {
    template <typename Pack>
    static inline void Go() {
        cout << setw(FIELD_WIDTH) << sizeof(typename Pack::Randomness);
    }
    static inline void Stop() { cout << endl; }
};

struct BenchPackWorker {
    template <typename Pack>
    static inline void Go(const typename Pack::Word *input, uint32_t length,
                          int repeat) {
        timing_stat_t t = Bench<Pack>(&input[0], length, repeat);

        if (t.wrong_answer) {
            cout << "BUG";
        } else {
            cout << setw(FIELD_WIDTH) << fixed << setprecision(2) << t.avg_opc;
        }
    }
    static inline void Stop() { cout << endl; }
};

template <typename Pack, typename... Packs>
void RunSizedBench(uint32_t length, int repeat) {
    vector<typename Pack::Word> input(length);
    for (auto &i : input) {
        i = get64rand();
    }
    cout << setw(FIRST_FIELD_WIDTH) << length;
    ForEachT<Pack, Packs...>::template Go<BenchPackWorker>(&input[0], length,
                                                           repeat);
}

template <typename Pack, typename... Packs>
void basic(const vector<uint32_t> &lengths, int repeat) {
    cout << numeric_limits<typename Pack::Word>::digits << " bit hash functions"
         << endl;
    cout << setw(FIRST_FIELD_WIDTH) << "size \\ hash fn";

    ForEachT<Pack, Packs...>::template Go<NameWorker>();
    cout << setw(FIRST_FIELD_WIDTH) << "rand size";

    ForEachT<Pack, Packs...>::template Go<SizeWorker>();
    for (const auto length : lengths) {
        RunSizedBench<Pack, Packs...>(length, repeat);
    }
}

#include <sys/resource.h>

int main() {
    int repeat = 1000;
    if (global_rdtsc_overhead == UINT64_MAX) {
        RDTSC_SET_OVERHEAD(repeat);
    }

    printf("We report the time (in cycles) necessary to hash a word.\n");
    printf("Size is reported in bytes.\n");
     printf("zobrist is 3-wise ind., linear is 2-wise ind., quadratic is 3-wise "
           "ind., cubic is 4-wise ind.\n");
    printf("Keys are flushed at the beginning of each run.\n");
    const vector<uint32_t> sizes{10, 20, 100, 1000, 10000, 100000,1000000};
    basic<Zobrist64Pack,WZobrist64Pack, ZobristTranspose64Pack, ThorupZhang64Pack, MultiplyShift64Pack,
          ClLinear64Pack, ClQuadratic64Pack, ClFastQuadratic64Pack, ClCubic64Pack, ClQuartic64Pack, ThorupZhangCWLinear64Pack, ThorupZhangCWQuadratic64Pack, ThorupZhangCWCubic64Pack>(sizes, repeat);

    basic<Zobrist32Pack, WZobrist32Pack, ThorupZhang32Pack, MultiplyShift32Pack, ClLinear32Pack, ClFastQuadratic32Pack, CWQuad32Pack, ThorupZhangCWLinear32Pack, ThorupZhangCWQuadratic32Pack, ThorupZhangCWCubic32Pack>(sizes, repeat);

    printf("Large runs are beneficial to tabulation-based hashing because they "
           "amortize cache faults.\n");
    printf("The 32-bit CL hash functions generate a pair of 32-bit hash values.\n");

}
