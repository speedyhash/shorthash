#ifndef SHORTHASH_UTIL_H
#define SHORTHASH_UTIL_H

#include <stdint.h>
#include <stdlib.h>

typedef unsigned __int128 uint128_t;


// helper function
static inline uint64_t __swapbits64(uint64_t p, uint64_t m, int k) {
  uint64_t q = ((p>>k)^p)&m;
  return p^q^(q<<k);
}

// knuth bit reversal https://matthewarcus.wordpress.com/2012/11/18/reversing-a-64-bit-word/
static uint64_t kbitreverse64 (uint64_t n) {
  static const uint64_t m0 = 0x5555555555555555LLU;
  static const uint64_t m1 = 0x0300c0303030c303LLU;
  static const uint64_t m2 = 0x00c0300c03f0003fLLU;
  static const uint64_t m3 = 0x00000ffc00003fffLLU;
  n = ((n>>1)&m0) | (n&m0)<<1;
  n = __swapbits64(n, m1, 4);
  n = __swapbits64(n, m2, 8);
  n = __swapbits64(n, m3, 20);
  n = (n >> 34) | (n << 30);
  return n;
}

static uint32_t get16rand() {
  return rand() >> 7;
}

static uint32_t get32rand() {
    return (((uint32_t)rand() << 0) & 0x0000FFFFul) |
           (((uint32_t)rand() << 16) & 0xFFFF0000ul);
}

static uint64_t get64rand() {
    return (((uint64_t)get32rand()) << 32) | get32rand();
}


static uint128_t get128rand() {
    return (((uint128_t)get64rand()) << 64) | get64rand();
}

#endif
