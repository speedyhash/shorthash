#include <stdio.h>
#include <stdlib.h>

#include "tabulated.h"
#include "clhash.h"

#define RDTSC_START(cycles)                                                   \
    do {                                                                      \
        register unsigned cyc_high, cyc_low;                                  \
        __asm volatile(                                                       \
            "cpuid\n\t"                                                       \
            "rdtsc\n\t"                                                       \
            "mov %%edx, %0\n\t"                                               \
            "mov %%eax, %1\n\t"                                               \
            : "=r"(cyc_high), "=r"(cyc_low)::"%rax", "%rbx", "%rcx", "%rdx"); \
        (cycles) = ((uint64_t)cyc_high << 32) | cyc_low;                      \
    } while (0)

#define RDTSC_FINAL(cycles)                                                   \
    do {                                                                      \
        register unsigned cyc_high, cyc_low;                                  \
        __asm volatile(                                                       \
            "rdtscp\n\t"                                                      \
            "mov %%edx, %0\n\t"                                               \
            "mov %%eax, %1\n\t"                                               \
            "cpuid\n\t"                                                       \
            : "=r"(cyc_high), "=r"(cyc_low)::"%rax", "%rbx", "%rcx", "%rdx"); \
        (cycles) = ((uint64_t)cyc_high << 32) | cyc_low;                      \
    } while (0)

static __attribute__ ((noinline))
uint64_t rdtsc_overhead_func(uint64_t dummy) {
    return dummy;
}

uint64_t global_rdtsc_overhead = (uint64_t) UINT64_MAX;

#define RDTSC_SET_OVERHEAD(test, repeat)			      \
  do {								      \
    uint64_t cycles_start, cycles_final, cycles_diff;		      \
    uint64_t min_diff = UINT64_MAX;				      \
    for (int i = 0; i < repeat; i++) {			      \
      __asm volatile("" ::: /* pretend to clobber */ "memory");	      \
      RDTSC_START(cycles_start);				      \
      test;							      \
      RDTSC_FINAL(cycles_final);                                       \
      cycles_diff = (cycles_final - cycles_start);		      \
      if (cycles_diff < min_diff) min_diff = cycles_diff;	      \
    }								      \
    global_rdtsc_overhead = min_diff;				      \
    printf("rdtsc_overhead set to %d\n", (int)global_rdtsc_overhead);     \
  } while (0)							      \


/*
 * Prints the best number of operations per cycle where
 * test is the function call,  repeat is the number of times we should repeat and size is the
 * number of operations represented by test.
 */
#define BEST_TIME(test, expected, repeat,  size)                           \
    do {                                                              \
        if (global_rdtsc_overhead == UINT64_MAX) {                    \
           RDTSC_SET_OVERHEAD(rdtsc_overhead_func(1), repeat);        \
        }                                                             \
        printf("%s: ", #test);                                        \
        fflush(NULL);                                                 \
        uint64_t cycles_start, cycles_final, cycles_diff;             \
        uint64_t min_diff = (uint64_t)-1;                             \
        int wrong_answer = 0;                                         \
        for (int i = 0; i < repeat; i++) {                            \
            __asm volatile("" ::: /* pretend to clobber */ "memory"); \
            RDTSC_START(cycles_start);                                \
            if (test != expected) wrong_answer = 1;                     \
            RDTSC_FINAL(cycles_final);                                \
            cycles_diff = (cycles_final - cycles_start);              \
            if (cycles_diff < min_diff) min_diff = cycles_diff;       \
        }                                                             \
        min_diff -= global_rdtsc_overhead;                            \
        uint64_t S = (uint64_t)size;                                  \
        float cycle_per_op = (min_diff) / (float)S;                   \
        float bytes_per_cycle = (float) S / (float) (min_diff);       \
        printf("size = %d,  %.2f cycles per byte or %.2f bytes per cycle ", size, cycle_per_op, bytes_per_cycle);           \
        if (wrong_answer) printf(" [ERROR]");                         \
        printf("\n");                                                 \
        fflush(NULL);                                                 \
    } while (0)


uint64_t demo_zobrist(uint64_t * array, uint32_t length, zobrist_t * k) {
    uint64_t sum = 0;
    for(uint32_t x = 0; x < length; ++x) {
      sum += zobrist(array[x],k);
    }
    return sum;
}

uint64_t demo_cl_linear(uint64_t * array, uint32_t length, cl_linear_t * k) {
    uint64_t sum = 0;
    for(uint32_t x = 0; x < length; ++x) {
      sum += cl_linear(array[x],k);
    }
    return sum;
}

uint64_t demo_cl_quadratic(uint64_t * array, uint32_t length, cl_quadratic_t * k) {
    uint64_t sum = 0;
    for(uint32_t x = 0; x < length; ++x) {
      sum += cl_quadratic(array[x],k);
    }
    return sum;
}

uint64_t demo_cl_cubic(uint64_t * array, uint32_t length, cl_cubic_t * k) {
    uint64_t sum = 0;
    for(uint32_t x = 0; x < length; ++x) {
      sum += cl_cubic(array[x],k);
    }
    return sum;
}


int main() {
    uint32_t length = 1000;
    printf("We will construct an array of %d words (using %d bytes), to be hashed.\n", (int) length, (int) ( length * sizeof(uint64_t)) );
    cl_linear_t cl_lineark;
    cl_linear_init(& cl_lineark);
   
    cl_quadratic_t cl_quadratick;
    cl_quadratic_init(& cl_quadratick);
 
    cl_cubic_t cl_cubick;
    cl_cubic_init(& cl_cubick);


    zobrist_t zobristk;
    zobrist_init(&  zobristk);

    printf("sizeof(cl_lineark) = %d, sizeof(cl_quadratick) = %d, sizeof(cl_cubick) = %d,  sizeof(zobristk) = %d \n",
          (int) sizeof(cl_lineark), (int) sizeof(cl_quadratick), (int) sizeof(cl_cubick), (int) sizeof(zobristk));

    uint64_t * array = malloc(sizeof(uint64_t) * length);
    for(uint32_t i = 0; i < length; ++i) {
      array[i] = get64rand();
    }
    uint64_t expected;
    uint32_t size = length;
    int repeat = 500;

    expected = demo_zobrist(array,length, &zobristk);
    BEST_TIME(demo_zobrist(array,length, &zobristk), expected, repeat,  size);

    expected = demo_cl_linear(array,length, &cl_lineark);
    BEST_TIME(demo_cl_linear(array,length, &cl_lineark), expected, repeat,  size);

    expected = demo_cl_quadratic(array,length, &cl_quadratick);
    BEST_TIME(demo_cl_quadratic(array,length, &cl_quadratick), expected, repeat,  size);

    expected = demo_cl_cubic(array,length, &cl_cubick);
    BEST_TIME(demo_cl_cubic(array,length, &cl_cubick), expected, repeat,  size);




    free(array);
}
