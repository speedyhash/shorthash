#ifndef SHORTHASH_LINEAR_H
#define SHORTHASH_LINEAR_H

#include <immintrin.h>

typedef struct { uint64_t rand[64]; } Linear64Randomness;

void Linear64Init(Linear64Randomness *r) {
    for (int i = 0; i < 64; ++i) {
        r->rand[i] = get64rand();
    }
}

uint64_t Linear64(uint64_t val, const Linear64Randomness *r) {
    uint64_t result = 0;
    for (int i = 0; i < 64; ++i) {
        const uint64_t p = __builtin_parityll(val & r->rand[i]);
        result |= p;
        result <<= 1;
    }
    return result;
}

typedef struct { __m128i rand; } Toeplitz64Randomness;

void Toeplitz64Init(Toeplitz64Randomness *r) {
  r->rand = (__m128i)get128rand();
}

uint64_t Toeplitz64(uint64_t val, const Toeplitz64Randomness *r) {
    const __m128i in = _mm_set1_epi64x(val);
    const __m128i lo = _mm_clmulepi64_si128(in, r->rand, 0x00);
    const __m128i hi = _mm_clmulepi64_si128(in, r->rand, 0x10);
    const uint64_t x = _mm_extract_epi64(lo, 0);
    const uint64_t y = _mm_extract_epi64(hi, 1) << 1;
    const uint64_t z = _mm_extract_epi64(hi, 0) >> 63;
    return x ^ (y | z);
}

#endif
