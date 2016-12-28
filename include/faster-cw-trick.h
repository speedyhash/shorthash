#ifndef SHORTHASH_FASTER_CW_TRICK_H
#define SHORTHASH_FASTER_CW_TRICK_H


#include "util.h"

/**
The thorup-zhang implementation of polynomial hashing
module 1<<89 - 1 may not be optimally fast. This is a
faster version.
*/




// this "almost" computes the modulo with 1<<89 - 1
// the "almost" comes from the fact that the result may overflow by one bit
__uint128_t almost_mersenne_modulo(__uint128_t x) {
  const int power = 89;
  const __uint128_t prime = ( (__uint128_t)(1) << power ) -1;
  return (x & prime) + (x >> power);
}


//  compute a*x +b mod Prime 89 where a and b are in [0,1<<89 -1)
__uint128_t mersennemult(__uint128_t a, uint64_t x, __uint128_t b) {
   const int power = 89;
   const __uint128_t prime = ( (__uint128_t)(1) << power ) -1;
  // result of x * (uint64_t) a fits in 128 bits, we compute "almost_mersenne_modulo"
  // so that there will be no overflow later
  __uint128_t result = almost_mersenne_modulo((__uint128_t)x*(uint64_t)a);
  // next we have something tricky to do, we need to multiply
  // a >> 64 with x and then compute the modulo
  __uint128_t messy = (__uint128_t)x*(a>>64); // result fully fits in 128  bits
  // in fact a < (1<<89 -1), so a>>64 is smaller than (1<<25 -1)
  // so that a * x is smaller than 1<<(25+64)-1
  const int highpower = (89 - 64);//25
  const uint64_t highprime = (UINT64_C(1) << highpower ) -  1;
  // we could almost do result += ((messy & highprime) + (messy >> highpower) )<< 64 but it might overflow!!!
  result +=  (messy & highprime) << 64  ;
  messy = messy >> highpower ; // top 25 bits are zero
  result += ( (messy & highprime)  + (messy >> highpower) ) << 64  ;
  // ok, now add the constant
  result += b;
  // next we compute the actual modulo
  result = almost_mersenne_modulo(result);
  // final reduction step
  if(result >= prime) result -= prime;
  return result;
}



typedef struct FasterCWLinear64_s {
    __uint128_t A;
    __uint128_t B;
} FasterCWLinear64_t;

typedef struct FasterCWQuadratic64_s {
    __uint128_t A;
    __uint128_t B;
    __uint128_t C;
} FasterCWQuadratic64_t;

typedef struct FasterCWCubic64_s {
    __uint128_t A;
    __uint128_t B;
    __uint128_t C;
    __uint128_t D;
} FasterCWCubic64_t;

__attribute__((always_inline))
inline void FasterCWLinear64Init(FasterCWLinear64_t * k) {
    for(int i = 0; i < 3; ++i) {
        k->A=get64rand();
        k->B=get64rand();
    }
}

__attribute__((always_inline))
inline void FasterCWQuadratic64Init(FasterCWQuadratic64_t * k) {
    for(int i = 0; i < 3; ++i) {
        k->A=get64rand();
        k->B=get64rand();
        k->C=get64rand();
    }
}


__attribute__((always_inline))
inline void FasterCWCubic64Init(FasterCWCubic64_t * k) {
    for(int i = 0; i < 3; ++i) {
        k->A=get64rand();
        k->B=get64rand();
        k->C=get64rand();
        k->D=get64rand();
    }
}


/* cW trick for 64-bit key x with prime 2ˆ89-1 */
__attribute__((always_inline))
inline uint64_t FasterCWLinear64(uint64_t x, const FasterCWLinear64_t * k) {
    return mersennemult(k->A,x,k->B);
}


/* cW trick for 64-bit key x with prime 2ˆ89-1 */
__attribute__((always_inline))
inline uint64_t FasterCWQuadratic64(uint64_t x, const FasterCWQuadratic64_t * k) {
    __uint128_t r = mersennemult(k->A,x,k->B);
    return mersennemult(r,x,k->C);
}


/* cW trick for 64-bit key x with prime 2ˆ89-1 */
__attribute__((always_inline))
inline uint64_t FasterCWCubic64(uint64_t x, const FasterCWCubic64_t * k) {
    __uint128_t r = mersennemult(k->A,x,k->B);
    r = mersennemult(r,x,k->C);
    return mersennemult(r,x,k->D);
}



#endif
