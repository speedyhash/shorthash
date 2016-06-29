#ifndef SHORTHASH_CLHASH_H
#define SHORTHASH_CLHASH_H

#include <stdint.h>

#include "util.h"

//////////////////////////
/// next line would be the "right thing to do" but instead we include
/// just the SSE/AVX dependencies we need otherwise it leads to a build error
/// with Intel compilers.
//////////////////////////
//#include <x86intrin.h> // can't do that on Intel compiler.
#include <wmmintrin.h>
#include <pmmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>
///////// End of compatibility hack.




// modulo reduction to 64-bit value. The high 64 bits contain garbage, call _mm_cvtsi128_si64 to extract
// the meaningful 64-bit
static inline __m128i reduction64_si128( __m128i A) {

    //const __m128i C = _mm_set_epi64x(1U,(1U<<4)+(1U<<3)+(1U<<1)+(1U<<0)); // C is the irreducible poly. (64,4,3,1,0)
    const __m128i C = _mm_cvtsi64_si128((1U<<4)+(1U<<3)+(1U<<1)+(1U<<0));
    __m128i Q2 = _mm_clmulepi64_si128( A, C, 0x01);
    const __m128i shuffle = _mm_setr_epi8(0, 27, 54, 45, 108, 119, 90, 65, 216, 195, 238, 245, 180, 175, 130, 153);
    __m128i Q3 = _mm_shuffle_epi8(shuffle,
                                  _mm_srli_si128(Q2,8));
    __m128i Q4 = _mm_xor_si128(Q2,A);
    const __m128i final = _mm_xor_si128(Q3,Q4);
    return final;/// WARNING: HIGH 64 BITS CONTAIN GARBAGE
}




typedef struct cl_linear_s {
  __m128i multiplier; // we only use lower 64-bit
  __m128i constant; // high 64-bit should be zero
} cl_linear_t;

void cl_linear_init(cl_linear_t * k) {
  k->multiplier = _mm_cvtsi64_si128(get64rand());
  k->constant = _mm_cvtsi64_si128(get64rand());
}

// this simply computes A x+B modulo
uint64_t cl_linear(uint64_t x, cl_linear_t * t) {
  __m128i inputasvector = _mm_cvtsi64_si128(x);
  __m128i product = _mm_clmulepi64_si128( inputasvector, t->multiplier, 0x00);
  __m128i productplusconstant = _mm_xor_si128( product, t->constant);
  return _mm_cvtsi128_si64(reduction64_si128(productplusconstant));
}

typedef struct cl_quadratic_s {
  __m128i multiplier;
  __m128i constant; // high 64-bit should be zero
} cl_quadratic_t;

void cl_quadratic_init(cl_quadratic_t * k) {
  k->multiplier = _mm_set_epi64x(get64rand(), get64rand());
  k->constant = _mm_cvtsi64_si128(get64rand());
}

// this simply computes   A x^2 + Bx +C modulo
// should be 3-wise ind.
uint64_t cl_quadratic(uint64_t x, cl_quadratic_t * t) {
  __m128i inputasvector = _mm_cvtsi64_si128(x);
  __m128i inputsquare = reduction64_si128(_mm_clmulepi64_si128(inputasvector,inputasvector,0x00));
  __m128i product1 = _mm_clmulepi64_si128( inputasvector, t->multiplier, 0x00);
  __m128i product2 = _mm_clmulepi64_si128( inputsquare, t->multiplier, 0x01);
  __m128i sum = _mm_xor_si128(product1,t->constant);
  sum = _mm_xor_si128(sum, product2);
  __m128i answer = reduction64_si128(sum);
  return _mm_cvtsi128_si64(answer);
}

typedef struct cl_cubic_s {
  __m128i multiplier1;
  __m128i multiplier2;
   __m128i constant; // high 64-bit should be zero
} cl_cubic_t;

void cl_cubic_init(cl_cubic_t * k) {
  k->multiplier1 = _mm_set_epi64x(get64rand(), get64rand());
  k->multiplier2 = _mm_cvtsi64_si128(get64rand());
  k->constant = _mm_cvtsi64_si128(get64rand());
}

// this simply computes   A x^3 + B x^2 + C x + D modulo
// should be 4-wise ind.
uint64_t cl_cubic(uint64_t x, cl_cubic_t * t) {
  __m128i inputasvector = _mm_cvtsi64_si128(x);
  __m128i inputsquare = reduction64_si128(_mm_clmulepi64_si128(inputasvector,inputasvector,0x00));
  __m128i inputcube = reduction64_si128(_mm_clmulepi64_si128(inputsquare,inputasvector,0x00));
  __m128i product1 = _mm_clmulepi64_si128( inputasvector, t->multiplier1, 0x00);
  __m128i product2 = _mm_clmulepi64_si128( inputsquare, t->multiplier1, 0x01);
  __m128i product3 = _mm_clmulepi64_si128( inputcube, t->multiplier2, 0x00);
  __m128i sum1 = _mm_xor_si128(product1,t->constant);
  __m128i sum2 = _mm_xor_si128(product2,product3);
  __m128i sum = _mm_xor_si128(sum1, sum2);
  __m128i answer = reduction64_si128(sum);
  return _mm_cvtsi128_si64(answer);
}

#endif
