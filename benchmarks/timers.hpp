#ifndef SHORTHASH_BENCHMARKS_TIMERS_HPP
#define SHORTHASH_BENCHMARKS_TIMERS_HPP

#include <cstdint>

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

#endif
