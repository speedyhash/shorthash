#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C" {
#include "tabulated.h"
#include "clhash.h"
}

inline uint64_t RDTSC_START() {
    register unsigned cyc_high, cyc_low;
    __asm volatile("cpuid\n\t"
                   "rdtsc\n\t"
                   "mov %%edx, %0\n\t"
                   "mov %%eax, %1\n\t"
                   : "=r"(cyc_high), "=r"(cyc_low)::"%rax", "%rbx", "%rcx",
                     "%rdx");
    return ((uint64_t)cyc_high << 32) | cyc_low;
}

inline uint64_t RDTSC_FINAL() {
    register unsigned cyc_high, cyc_low;
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
inline void BEST_TIME(const T &hasher, int repeat, int size) {
    if (global_rdtsc_overhead == UINT64_MAX) {
        RDTSC_SET_OVERHEAD(repeat);
    }
    fflush(NULL);
    uint64_t cycles_start, cycles_final, cycles_diff;
    uint64_t min_diff = (uint64_t)-1;
    int wrong_answer = 0;
    for (int i = 0; i < repeat; i++) {
        __asm volatile("" ::: /* pretend to clobber */ "memory");
        hasher.Restart();
        cycles_start = RDTSC_START();
        if (hasher.Hash() != hasher.expected_)
            wrong_answer = 1;
        cycles_final = RDTSC_FINAL();
        cycles_diff = (cycles_final - cycles_start);
        if (cycles_diff < min_diff)
            min_diff = cycles_diff;
    }
    min_diff -= global_rdtsc_overhead;
    uint64_t S = (uint64_t)size;
    float cycle_per_op = (min_diff) / (float)S;
    printf("size = %d,  %.2f cycles per word ", size, cycle_per_op);
    if (wrong_answer)
        printf(" [ERROR]");
    printf("\n");
    fflush(NULL);
}

template <typename HashDataType, typename HashValueType,
          HashValueType HashFunction(HashValueType, const HashDataType *)>
struct HashBench {
  inline void Restart() const {
    flush(&k_, sizeof(k_));
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

void flush(const void *b, size_t length) {
    char *B = (char *)b;
    for (uint32_t k = 0; k < length; k += 64) {
        __builtin_ia32_clflush(B + k);
    }
}

void basic(uint32_t length) {
    printf("Testing 64-bit hashing.\n");
    printf("We will construct an array of %d words (using %d bytes), to be "
           "hashed.\n",
           (int)length, (int)(length * sizeof(uint64_t)));
    printf("Keys are flushed at the beginning of each run.\n");
    cl_linear_t cl_lineark;
    cl_linear_init(&cl_lineark);

    cl_quadratic_t cl_quadratick;
    cl_quadratic_init(&cl_quadratick);

    cl_cubic_t cl_cubick;
    cl_cubic_init(&cl_cubick);

    zobrist_t zobristk;
    zobrist_init(&zobristk);

    printf("sizeof(cl_lineark) = %d, sizeof(cl_quadratick) = %d, "
           "sizeof(cl_cubick) = %d,  sizeof(zobristk) = %d \n",
           (int)sizeof(cl_lineark), (int)sizeof(cl_quadratick),
           (int)sizeof(cl_cubick), (int)sizeof(zobristk));

    uint64_t *array = (uint64_t *)malloc(sizeof(uint64_t) * length);
    for (uint32_t i = 0; i < length; ++i) {
        array[i] = get64rand();
    }

    uint32_t size = length;
    int repeat = 500;

    printf("zobrist: ");
    HashBench<zobrist_t, uint64_t, zobrist> demo_zobrist(array, length,
                                                         &zobristk);
    BEST_TIME(demo_zobrist, repeat, size);

    printf("cl_linear: ");
    HashBench<cl_linear_t, uint64_t, cl_linear> demo_linear(array, length,
                                                            &cl_lineark);
    BEST_TIME(demo_linear, repeat, size);

    printf("cl_quadratic: ");
    HashBench<cl_quadratic_t, uint64_t, cl_quadratic> demo_quadratic(
        array, length, &cl_quadratick);
    BEST_TIME(demo_quadratic, repeat, size);

    printf("cl_cubic: ");
    HashBench<cl_cubic_t, uint64_t, cl_cubic> demo_cubic(array, length,
                                                         &cl_cubick);
    BEST_TIME(demo_cubic, repeat, size);

    printf("zobrist is 3-wise ind., linear is 2-wise ind., quadratic is 3-wise "
           "ind., cubic is 4-wise ind.\n");

    free(array);
    printf("\n");
}

void basic32(uint32_t length) {
    printf("Testing 32-bit hashing.\n");
    printf("We will construct an array of %d words (using %d bytes), to be "
           "hashed.\n",
           (int)length, (int)(length * sizeof(uint32_t)));
    printf("Keys are flushed at the beginning of each run.\n");
    cl_quadratic_t cl_quadratick;
    cl_quadratic32_init(&cl_quadratick);

    cl_linear_t cl_lineark;
    cl_linear32_init(&cl_lineark);

    zobrist32_t zobristk;
    zobrist32_init(&zobristk);

    printf(" sizeof(cl_lineark) = %d, sizeof(cl_quadratick) = %d, "
           "sizeof(zobristk) = %d \n",
           (int)sizeof(cl_lineark), (int)sizeof(cl_quadratick),
           (int)sizeof(zobristk));

    uint32_t *array = (uint32_t *)malloc(sizeof(uint32_t) * length);
    for (uint32_t i = 0; i < length; ++i) {
        array[i] = get32rand();
    }

    uint32_t size = length;
    int repeat = 500;

    printf("zobrist: ");
    HashBench<zobrist32_t, uint32_t, zobrist32> demo_zobrist(array, length,
                                                         &zobristk);
    BEST_TIME(demo_zobrist, repeat, size);

    printf("cl_linear: ");
    HashBench<cl_linear_t, uint32_t, cl_linear32> demo_linear(array, length,
                                                            &cl_lineark);
    BEST_TIME(demo_linear, repeat, size);

    printf("cl_quadratic: ");
    HashBench<cl_quadratic_t, uint32_t, cl_quadratic32> demo_quadratic(
        array, length, &cl_quadratick);
    BEST_TIME(demo_quadratic, repeat, size);

    printf("zobrist is 3-wise ind., linear is 2-wise ind., quadratic is 3-wise "
           "ind.\n");

    free(array);
    printf("\n");
}

int main() {
    printf("=======");
    basic(10);
    basic(20);
    basic(100);
    basic(1000);
    printf("=======");
    basic32(10);
    basic32(20);
    basic32(100);
    basic32(1000);

    printf("Large runs are beneficial to tabulation-based hashing because they "
           "amortize cache faults.\n");
}
