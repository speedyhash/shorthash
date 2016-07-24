#ifndef SHORTHASH_MULTIPLY_SHIFT_H
#define SHORTHASH_MULTIPLY_SHIFT_H

#include <stdint.h>

#include "util.h"



typedef struct bitmixing_s {
    uint64_t multiplier;
} bitmixing_t;

void bitmixing_init(bitmixing_t *k) {
    k->multiplier = UINT64_C(15185512759463952534);
}

// this simply computes A x+B modulo
__attribute__((always_inline))
inline uint64_t bitmixing(uint64_t x, const bitmixing_t *t) {
  return (t->multiplier * x ) >> 32;
}


/**
* The compiler is probably smart enough to do it right, but if not:

uint64_t mulshift(uint64_t u, uint64_t v) {
    uint64_t h;
    __asm__ ("mulq %[v]\n"
             :   [h] "+d" (h)    : [u] "a" (u), [v] "r" (v)  :"rdx","cc" );
    return h;
}

*/


// Multiply-shift is a 2-independent method of hashing without the need for
// prime numbers. See M. Dietzfelbinger. "Universal hashing and k-wise
// independent random variables via integer arithmetic without primes." In Proc.
// 13th STACS, LNCS 1046, pages 569-580, 1996.
typedef struct {
  uint128_t mult, add;
} MultiplyShift64Randomness;

void MultiplyShift64Init(MultiplyShift64Randomness *x) {
  x->mult = get128rand();
  x->add = get128rand();
}

__attribute__((always_inline))
inline uint64_t MultiplyShift64(uint64_t in, const MultiplyShift64Randomness * rand) {
  return ((((uint128_t)in) * rand->mult) + rand->add) >> 64;
}

typedef struct {
  uint64_t mult;
} UnivMultiplyShift64Randomness;

void UnivMultiplyShift64Init(UnivMultiplyShift64Randomness *x) {
  x->mult = 1 | get64rand();
}

__attribute__((always_inline))
inline uint64_t UnivMultiplyShift64(uint64_t in, const UnivMultiplyShift64Randomness * rand) {
  return in * rand->mult;
}

typedef struct {
  uint128_t mult1, mult2;
} MultiplyTwice64Randomness;

void MultiplyTwice64Init(MultiplyTwice64Randomness *x) {
  x->mult1 = get128rand();
  x->mult2 = get128rand();
}

__attribute__((always_inline)) inline uint64_t
    MultiplyTwice64(uint64_t in, const MultiplyTwice64Randomness *rand) {
    return ((((uint128_t)in) * rand->mult1) >> 64) ^
           ((((uint128_t)in) * rand->mult2) >> 64);
}

typedef struct {
  uint128_t mults[3];
} MultiplyThrice64Randomness;

void MultiplyThrice64Init(MultiplyThrice64Randomness *x) {
    for (int i = 0; i < 3; ++i) {
        x->mults[i] = get128rand();
    }
}

__attribute__((always_inline)) inline uint64_t
    MultiplyThrice64(uint64_t in, const MultiplyThrice64Randomness *rand) {
    uint64_t ans = (((uint128_t)in) * rand->mults[0]) >> 64;
    for (int i = 1; i < 3; ++i) {
        ans ^= (((uint128_t)in) * rand->mults[i]) >> 64;
    }
    return ans;
}

typedef struct {
  uint64_t mult, add;
} MultiplyShift32Randomness;

void MultiplyShift32Init(MultiplyShift32Randomness *x) {
  x->mult = get64rand();
  x->add = get64rand();
}

__attribute__((always_inline))
inline uint32_t MultiplyShift32(uint32_t in, const MultiplyShift32Randomness * rand) {
  return ((((uint64_t)in) * rand->mult) + rand->add) >> 32;
}

typedef struct {
  uint16_t mult;
} MultiplyOnly8Randomness;

void MultiplyOnly8Init(MultiplyOnly8Randomness *x) {
  x->mult = get16rand();
}

__attribute__((always_inline))
inline uint8_t MultiplyOnly8(uint8_t in, const MultiplyOnly8Randomness * rand) {
    return (((uint16_t)in) * rand->mult) >> 8;
}


#endif // SHORTHASH_MULTIPLY_SHIFT_H
