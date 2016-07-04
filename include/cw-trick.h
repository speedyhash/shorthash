#ifndef SHORTHASH_CW_TRICK_H
#define SHORTHASH_CW_TRICK_H

#include "util.h"

const uint8_t CW_PRIME_POWER = 61;
const uint64_t CW_PRIME = (((uint64_t)1) << CW_PRIME_POWER) - 1;

uint64_t cwmod(uint128_t x) {
    uint64_t rmq = x & CW_PRIME;
    uint128_t q = (x - rmq) >> CW_PRIME_POWER;
    uint64_t r = rmq + q;
    return r;
}

typedef struct {
  uint32_t a,b,c;
} CWRandomQuad32;

void CWRandomQuad32Init(CWRandomQuad32 * x) {
  x->a = get32rand();
  x->b = get32rand();
  x->c = get32rand();
}

uint32_t CWQuad32(uint32_t x, const CWRandomQuad32 * r) {
    uint64_t ax = ((uint64_t)r->a) * ((uint64_t)x);      // 64 bits
    uint128_t axb = ((uint128_t)ax) + ((uint128_t)r->b); // 65 bits
    uint128_t axbx = axb * ((uint128_t)x);            // 97 bits
    uint128_t axbxc = axbx + ((uint128_t)r->c);          // 98 bits
    return cwmod(axbxc);
    //  (a*x + b)*x + c
}

#endif
