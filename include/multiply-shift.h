#ifndef SHORTHASH_MULTIPLY_SHIFT_H
#define SHORTHASH_MULTIPLY_SHIFT_H

#include <stdint.h>

#include "util.h"

// Multiply-shift is a 2-independent method of hashing without the need for
// prime numbers. See M. Dietzfelbinger. "Universal hashing and k-wise
// independent random variables via integer arithmetic without primes." In Proc.
// 13th STACS, LNCS 1046, pages 569–580, 1996.
typedef struct {
  uint128_t mult, add;
} MultiplyShift64Randomness;

void MultiplyShift64Init(MultiplyShift64Randomness *x) {
  x->mult = get128rand();
  x->add = get128rand();
}

uint64_t MultiplyShift64(uint64_t in, const MultiplyShift64Randomness * rand) {
  return ((((uint128_t)in) * rand->mult) + rand->add) >> 64;
}


typedef struct {
  uint64_t mult, add;
} MultiplyShift32Randomness;

void MultiplyShift32Init(MultiplyShift32Randomness *x) {
  x->mult = get64rand();
  x->add = get64rand();
}

uint32_t MultiplyShift32(uint32_t in, const MultiplyShift32Randomness * rand) {
  return ((((uint64_t)in) * rand->mult) + rand->add) >> 32;
}


#endif // SHORTHASH_MULTIPLY_SHIFT_H
