#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;

extern "C" {
#include "clhash.h"
#include "cw-trick.h"
#include "multiply-shift.h"
#include "tabulated.h"
}

inline uint64_t RDTSC_START() {
    unsigned cyc_high, cyc_low;
    __asm volatile("cpuid\n\t"
                   "rdtsc\n\t"
                   "mov %%edx, %0\n\t"
                   "mov %%eax, %1\n\t"
                   : "=r"(cyc_high), "=r"(cyc_low)::"%rax", "%rbx", "%rcx",
                     "%rdx");
    return ((uint64_t)cyc_high << 32) | cyc_low;
}

inline uint64_t RDTSC_FINAL() {
    unsigned cyc_high, cyc_low;
    __asm volatile("rdtscp\n\t"
                   "mov %%edx, %0\n\t"
                   "mov %%eax, %1\n\t"
                   "cpuid\n\t"
                   : "=r"(cyc_high), "=r"(cyc_low)::"%rax", "%rbx", "%rcx",
                     "%rdx");
    return ((uint64_t)cyc_high << 32) | cyc_low;
}

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
        cycles_diff = (cycles_final - cycles_start);
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
static const int FIELD_WIDTH = 16;

template <typename T>
inline timing_stat_t Bench(const typename T::Word *input, uint32_t length, int repeat) {
    typename T::Randomness randomness;
    T::InitRandomness(&randomness);

    repeat = repeat / length;
    HashBench<typename T::Randomness, typename T::Word, &T::HashFunction> demo(
        input, length, &randomness);
    return BEST_TIME(demo, repeat, length);
}

template <typename WordP, typename RandomnessP,
          void (*InitRandomnessP)(RandomnessP *),
          WordP (*HashFunctionP)(WordP, const RandomnessP *)>
struct GenericPack {
    typedef WordP Word;
    typedef RandomnessP Randomness;
    static inline void InitRandomness(Randomness *r) { InitRandomnessP(r); }
    static inline Word HashFunction(Word x, const Randomness *r) {
        return HashFunctionP(x, r);
    }
};

struct Zobrist64Pack
    : public GenericPack<uint64_t, zobrist_t, zobrist_init, zobrist> {
    static constexpr auto NAME = "Zobrist64";
};

struct ZobristTranspose64Pack
    : public GenericPack<uint64_t, zobrist_flat_t, zobrist_flat_init,
                         zobrist_flat_transpose> {
    static constexpr auto NAME = "Transposed64";
};

struct ClLinear64Pack
    : public GenericPack<uint64_t, cl_linear_t, cl_linear_init, cl_linear> {
    static constexpr auto NAME = "ClLinear64";
};

struct MultiplyShift64Pack
    : public GenericPack<uint64_t, MultiplyShift64Randomness,
                         MultiplyShift64Init, MultiplyShift64> {
    static constexpr auto NAME = "MultiplyShift64";
};

struct ClQuadratic64Pack : public GenericPack<uint64_t, cl_quadratic_t,
                                              cl_quadratic_init, cl_quadratic> {
    static constexpr auto NAME = "ClQuadratic64";
};

struct ClCubic64Pack
    : public GenericPack<uint64_t, cl_cubic_t, cl_cubic_init, cl_cubic> {
    static constexpr auto NAME = "ClCubic64";
};

struct ClQuartic64Pack
    : public GenericPack<uint64_t, cl_quartic_t, cl_quartic_init, cl_quartic> {
    static constexpr auto NAME = "ClQuartic64";
};

struct Zobrist32Pack
    : public GenericPack<uint32_t, zobrist32_t, zobrist32_init, zobrist32> {
    static constexpr auto NAME = "Zobrist32";
};

struct ClLinear32Pack
    : public GenericPack<uint32_t, cl_linear_t, cl_linear32_init, cl_linear32> {
    static constexpr auto NAME = "ClLinear32";
};

struct ClQuadratic32Pack
    : public GenericPack<uint32_t, cl_quadratic_t, cl_quadratic32_init,
                         cl_quadratic32> {
    static constexpr auto NAME = "ClQuadratic32";
};

struct ClCubic32Pack
    : public GenericPack<uint32_t, cl_cubic_t, cl_cubic32_init,
                         cl_cubic32> {
    static constexpr auto NAME = "ClCubic32";
};

struct ClQuartic32Pack
    : public GenericPack<uint32_t, cl_quartic_t, cl_quartic32_init,
                         cl_quartic32> {
    static constexpr auto NAME = "ClQuartic32";
};

struct CWQuad32Pack
    : public GenericPack<uint32_t, CWRandomQuad32, CWRandomQuad32Init,
                         CWQuad32> {
    static constexpr auto NAME = "CWQuad32";
};


template <typename... Pack> inline void BenchPack(...) { cout << endl; }



template <typename Pack, typename... Rest>
inline void BenchPack(const typename Pack::Word *input, uint32_t length,
                      int repeat) {
    timing_stat_t t = Bench<Pack>(&input[0], length, repeat);

    if(t.wrong_answer) {
      cout << "BUG";
    } else  {
      cout << setw(FIELD_WIDTH) << fixed << setprecision(2) << t.avg_opc ;
    }
    BenchPack<Rest...>(input, length, repeat);
}

template <typename... Pack> inline void NamePack(...) { cout << endl; }

template <typename Pack, typename... Rest>
inline void NamePack(typename Pack::Word dummy) {
    cout << setw(FIELD_WIDTH) << Pack::NAME;
    NamePack<Rest...>(dummy);
}

template <typename... Pack> inline void SizePack(...) { cout << endl; }

template <typename Pack, typename... Rest>
inline void SizePack(typename Pack::Word dummy) {
    cout << setw(FIELD_WIDTH) << sizeof(typename Pack::Randomness);
    SizePack<Rest...>(dummy);
}

template <typename Pack, typename... Packs>
void RunSizedBench(uint32_t length, int repeat) {
    vector<typename Pack::Word> input(length);
    for (auto &i : input) {
        i = get64rand();
    }
    cout << setw(FIRST_FIELD_WIDTH) << length;
    BenchPack<Pack, Packs...>(&input[0], length, repeat);
}

template <typename Pack, typename... Packs>
void basic(const vector<uint32_t> &lengths, int repeat) {
    cout << numeric_limits<typename Pack::Word>::digits << " bit hash functions"
         << endl;
    cout << setw(FIRST_FIELD_WIDTH) << "size \\ hash fn";
    NamePack<Pack, Packs...>(0);
    cout << setw(FIRST_FIELD_WIDTH) << "rand size";
    SizePack<Pack, Packs...>(0);
    for (const auto length : lengths) {
        RunSizedBench<Pack, Packs...>(length, repeat);
    }
}

int main() {
    int repeat = 100000;
    if (global_rdtsc_overhead == UINT64_MAX) {
        RDTSC_SET_OVERHEAD(repeat);
    }
    printf("zobrist is 3-wise ind., linear is 2-wise ind., quadratic is 3-wise "
           "ind., cubic is 4-wise ind.\n");
    printf("Keys are flushed at the beginning of each run.\n");
    //const vector<uint32_t> sizes{10, 20, 100, 1000, 10000, 100000};
    vector<uint32_t> sizes {5,6,7,8,9,10,11,12,
                            13,14,15,16,17,18,19,20, 50, 100, 500, 1000, 100000};
    basic<Zobrist64Pack, ZobristTranspose64Pack, MultiplyShift64Pack,
          ClLinear64Pack, ClQuadratic64Pack, ClCubic64Pack, ClQuartic64Pack>(sizes, repeat);

    basic<Zobrist32Pack, ClLinear32Pack, ClQuadratic32Pack, ClCubic32Pack,
          ClQuartic32Pack, CWQuad32Pack>(sizes, repeat);

    printf("Large runs are beneficial to tabulation-based hashing because they "
           "amortize cache faults.\n");
}
