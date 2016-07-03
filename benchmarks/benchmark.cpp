#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iomanip>
#include <iostream>

using namespace std;

extern "C" {
#include "tabulated.h"
#include "clhash.h"
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

/*
 * Prints the best number of operations per cycle where
 * test is the function call,  repeat is the number of times we should repeat
 * and size is the
 * number of operations represented by test.
 */
template <typename T>
inline float BEST_TIME(const T &hasher, int repeat, int size) {
    uint64_t cycles_start, cycles_final, cycles_diff;
    uint64_t min_diff = (uint64_t)-1;
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
    }
    if (wrong_answer) return -1;
    min_diff -= global_rdtsc_overhead;
    return min_diff / (float)size;
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

void basic(uint32_t length, int repeat) {
    cl_linear_t cl_lineark;
    cl_linear_init(&cl_lineark);

    cl_quadratic_t cl_quadratick;
    cl_quadratic_init(&cl_quadratick);

    cl_cubic_t cl_cubick;
    cl_cubic_init(&cl_cubick);

    zobrist_t zobristk;
    zobrist_init(&zobristk);

    zobrist_flat_t zobrist_flatk;
    zobrist_flat_init(&zobrist_flatk);

    static const int FIRST_FIELD_WIDTH = 20;
    static const int FIELD_WIDTH = 13;

    static bool first_run = true;
    if (first_run) {
        printf("Testing 64-bit hashing.\n");

        printf("sizeof(cl_lineark) = %d, sizeof(cl_quadratick) = %d, "
               "sizeof(cl_cubick) = %d,  sizeof(zobristk) = %d \n",
               (int)sizeof(cl_lineark), (int)sizeof(cl_quadratick),
               (int)sizeof(cl_cubick), (int)sizeof(zobristk));
        cout << setw(FIELD_WIDTH) << "array size \\ hash fn"
             << setw(FIELD_WIDTH) << "zobrist" << setw(FIELD_WIDTH)
             << "transposed" << setw(FIELD_WIDTH) << "cl_linear"
             << setw(FIELD_WIDTH) << "cl_quadratic" << setw(FIELD_WIDTH)
             << "cl_cubic" << endl;
        first_run = false;
    }

    uint64_t *array = (uint64_t *)malloc(sizeof(uint64_t) * length);
    for (uint32_t i = 0; i < length; ++i) {
        array[i] = get64rand();
    }

    uint32_t size = length;

    cout << setw(FIRST_FIELD_WIDTH) << length;
    HashBench<zobrist_t, uint64_t, zobrist> demo_zobrist(array, length,
                                                         &zobristk);
    cout << setw(FIELD_WIDTH) << fixed << setprecision(2)
         << BEST_TIME(demo_zobrist, repeat, size);

    HashBench<zobrist_flat_t, uint64_t, zobrist_flat_transpose>
        demo_zobrist_flat_transpose(array, length, &zobrist_flatk);
    cout << setw(FIELD_WIDTH)
         << BEST_TIME(demo_zobrist_flat_transpose, repeat, size);

    HashBench<cl_linear_t, uint64_t, cl_linear> demo_linear(array, length,
                                                            &cl_lineark);
    cout << setw(FIELD_WIDTH) << BEST_TIME(demo_linear, repeat, size);

    HashBench<cl_quadratic_t, uint64_t, cl_quadratic> demo_quadratic(
        array, length, &cl_quadratick);
    cout << setw(FIELD_WIDTH) << BEST_TIME(demo_quadratic, repeat, size);

    HashBench<cl_cubic_t, uint64_t, cl_cubic> demo_cubic(array, length,
                                                         &cl_cubick);
    cout << setw(FIELD_WIDTH) << BEST_TIME(demo_cubic, repeat, size);

    free(array);
    printf("\n");
}

void basic32(uint32_t length, int repeat) {
    cl_quadratic_t cl_quadratick;
    cl_quadratic32_init(&cl_quadratick);

    cl_linear_t cl_lineark;
    cl_linear32_init(&cl_lineark);

    zobrist32_t zobristk;
    zobrist32_init(&zobristk);

    static const int FIRST_FIELD_WIDTH = 20;
    static const int FIELD_WIDTH = 13;

    static bool first_run = true;
    if (first_run) {
        printf("Testing 32-bit hashing.\n");
        printf("sizeof(cl_lineark) = %d, sizeof(cl_quadratick) = %d, "
               "sizeof(zobristk) = %d \n",
               (int)sizeof(cl_lineark), (int)sizeof(cl_quadratick),
               (int)sizeof(zobristk));
        cout << setw(FIRST_FIELD_WIDTH) << "array size \\ hash fn"
             << setw(FIELD_WIDTH) << "zobrist" << setw(FIELD_WIDTH)
             << "cl_linear" << setw(FIELD_WIDTH) << "cl_quadratic" << endl;
        first_run = false;
    }

    uint32_t *array = (uint32_t *)malloc(sizeof(uint32_t) * length);
    for (uint32_t i = 0; i < length; ++i) {
        array[i] = get32rand();
    }

    uint32_t size = length;

    cout << setw(FIRST_FIELD_WIDTH) << length;
    HashBench<zobrist32_t, uint32_t, zobrist32> demo_zobrist(array, length,
                                                         &zobristk);
    cout << setw(FIELD_WIDTH) << fixed << setprecision(2)
         << BEST_TIME(demo_zobrist, repeat, size);

    HashBench<cl_linear_t, uint32_t, cl_linear32> demo_linear(array, length,
                                                            &cl_lineark);
    cout << setw(FIELD_WIDTH) << BEST_TIME(demo_linear, repeat, size);

    HashBench<cl_quadratic_t, uint32_t, cl_quadratic32> demo_quadratic(
        array, length, &cl_quadratick);
    cout << setw(FIELD_WIDTH) << BEST_TIME(demo_quadratic, repeat, size);

    free(array);
    printf("\n");
}

int main() {
    int repeat = 50000;
    if (global_rdtsc_overhead == UINT64_MAX) {
        RDTSC_SET_OVERHEAD(repeat);
    }
    printf("zobrist is 3-wise ind., linear is 2-wise ind., quadratic is 3-wise "
           "ind., cubic is 4-wise ind.\n");
    printf("Keys are flushed at the beginning of each run.\n");
    printf("=======");
    basic(10, repeat);
    basic(20, repeat);
    basic(100, repeat);
    basic(1000, repeat);

    printf("=======");
    basic32(10, repeat);
    basic32(20, repeat);
    basic32(100, repeat);
    basic32(1000, repeat);

    printf("Large runs are beneficial to tabulation-based hashing because they "
           "amortize cache faults.\n");
}
