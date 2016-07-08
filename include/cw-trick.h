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

__attribute__((always_inline))
inline uint32_t CWQuad32(uint32_t x, const CWRandomQuad32 * r) {
    uint64_t ax = ((uint64_t)r->a) * ((uint64_t)x);      // 64 bits
    uint128_t axb = ((uint128_t)ax) + ((uint128_t)r->b); // 65 bits
    uint128_t axbx = axb * ((uint128_t)x);            // 97 bits
    uint128_t axbxc = axbx + ((uint128_t)r->c);          // 98 bits
    return cwmod(axbxc);
    //  (a*x + b)*x + c
}





/***
Rest is from Appendix for
Tabulation Based 4-Universal Hashing with Applications to
Second Moment Estimation
**/


#define LOW(x) ((x)&0xFFFFFFFF) // extract lower 32 bits from INT64
#define HIGH(x) ((x)>>32) // extract higher 32 bits from INT64

const static uint64_t Prime61 = (((uint64_t)1)<<61) - 1;

/* Computes ax+b mod Prime, possibly plus 2*Prime,
exploiting the structure of Prime. */
static inline uint64_t MultAddPrime61(uint32_t x, uint64_t a, uint64_t b) {
    uint64_t a0,a1,c0,c1,c;
    a0 = LOW(a)*x;
    a1 = HIGH(a)*x;
    c0 = a0+(a1<<32);
    c1 = (a0>>32)+a1;
    c = (c0&Prime61)+(c1>>29)+b;
    return c;
}

typedef struct ThorupZhangCWLinear32_s {
    uint64_t A;
    uint64_t B;
} ThorupZhangCWLinear32_t;


typedef struct ThorupZhangCWQuadratic32_s {
    uint64_t A;
    uint64_t B;
    uint64_t C;
} ThorupZhangCWQuadratic32_t;


typedef struct ThorupZhangCWCubic32_s {
    uint64_t A;
    uint64_t B;
    uint64_t C;
    uint64_t D;
} ThorupZhangCWCubic32_t;

void ThorupZhangCWLinear32Init(ThorupZhangCWLinear32_t * k) {
    k->A=get64rand();
    k->B=get64rand();
}


void ThorupZhangCWQuadratic32Init(ThorupZhangCWQuadratic32_t * k) {
    k->A=get64rand();
    k->B=get64rand();
    k->C=get64rand();
}


void ThorupZhangCWCubic32Init(ThorupZhangCWCubic32_t * k) {
    k->A=get64rand();
    k->B=get64rand();
    k->C=get64rand();
    k->D=get64rand();
}

/* CWtrick for 32-bit key x with prime 2ˆ61-1 */
__attribute__((always_inline))
inline uint32_t ThorupZhangCWLinear32(uint32_t x, const ThorupZhangCWLinear32_t * k) {
    uint64_t h;
    h = MultAddPrime61(x,k->A,k->B);
    h = (h&Prime61)+(h>>61);
    if (h>=Prime61) h-=Prime61;
    return h;
}

/* CWtrick for 32-bit key x with prime 2ˆ61-1 */
__attribute__((always_inline))
inline uint32_t ThorupZhangCWQuadratic32(uint32_t x, const ThorupZhangCWQuadratic32_t * k) {
    uint64_t h;
    h = MultAddPrime61(MultAddPrime61(x,k->A,k->B),x,k->C);
    h = (h&Prime61)+(h>>61);
    if (h>=Prime61) h-=Prime61;
    return h;
}

/* CWtrick for 32-bit key x with prime 2ˆ61-1 */
__attribute__((always_inline))
inline uint32_t ThorupZhangCWCubic32(uint32_t x, const ThorupZhangCWCubic32_t * k) {
    uint64_t h;
    h = MultAddPrime61(MultAddPrime61(MultAddPrime61(x,k->A,k->B),x,k->C),x,k->D);
    h = (h&Prime61)+(h>>61);
    if (h>=Prime61) h-=Prime61;
    return h;
}

const static uint64_t Prime89_0 = (((uint64_t)1)<<32)-1;
const static uint64_t Prime89_1 = (((uint64_t)1)<<32)-1;
const static uint64_t Prime89_2 = (((uint64_t)1)<<25)-1;
const static uint64_t Prime89_21 = (((uint64_t)1)<<57)-1;

typedef uint64_t INT96[3];

/* Computes (r mod Prime89) mod 2ˆ64, exploiting the structure of Prime89 */
static inline uint64_t Mod64Prime89(INT96 r) {
    uint64_t r0, r1, r2;
    // r2r1r0 = r&Prime89 + r>>89
    r2 = r[2];
    r1 = r[1];
    r0 = r[0] + (r2>>25);
    r2 &= Prime89_2;
    return (r2 == Prime89_2 && r1 == Prime89_1 && r0 >= Prime89_0) ?
           (r0 - Prime89_0) : (r0 + (r1<<32));
}


/* Computes a 96-bit r s.t. r mod Prime89 == (ax+b) mod Prime89
exploiting the structure of Prime89. */
static inline void MultAddPrime89(INT96 r, uint64_t x, const INT96 a, const INT96 b) {
    uint64_t x1, x0, c21, c20, c11, c10, c01, c00;
    uint64_t d0, d1, d2, d3;
    uint64_t s0, s1, carry;
    x1 = HIGH(x);
    x0 = LOW(x);
    c21 = a[2]*x1;
    c11 = a[1]*x1;
    c01 = a[0]*x1;
    c20 = a[2]*x0;
    c10 = a[1]*x0;
    c00 = a[0]*x0;
    d0 = (c20>>25)+(c11>>25)+(c10>>57)+(c01>>57);
    d1 = (c21<<7);
    d2 = (c10&Prime89_21) + (c01&Prime89_21);
    d3 = (c20&Prime89_2) + (c11&Prime89_2) + (c21>>57);
    s0 = b[0] + LOW(c00) + LOW(d0) + LOW(d1);
    r[0] = LOW(s0);
    carry = HIGH(s0);
    s1 = b[1] + HIGH(c00) + HIGH(d0) + HIGH(d1) + LOW(d2) + carry;
    r[1] = LOW(s1);
    carry = HIGH(s1);
    r[2] = b[2] + HIGH(d2) + d3 + carry;
}
typedef struct ThorupZhangCWLinear64_s {
    INT96 A;
    INT96 B;
} ThorupZhangCWLinear64_t;

typedef struct ThorupZhangCWQuadratic64_s {
    INT96 A;
    INT96 B;
    INT96 C;
} ThorupZhangCWQuadratic64_t;

typedef struct ThorupZhangCWCubic64_s {
    INT96 A;
    INT96 B;
    INT96 C;
    INT96 D;
} ThorupZhangCWCubic64_t;

__attribute__((always_inline))
inline void ThorupZhangCWLinear64Init(ThorupZhangCWLinear64_t * k) {
    for(int i = 0; i < 3; ++i) {
        k->A[i]=get64rand();
        k->B[i]=get64rand();
    }
}

__attribute__((always_inline))
inline void ThorupZhangCWQuadratic64Init(ThorupZhangCWQuadratic64_t * k) {
    for(int i = 0; i < 3; ++i) {
        k->A[i]=get64rand();
        k->B[i]=get64rand();
        k->C[i]=get64rand();
    }
}


__attribute__((always_inline))
inline void ThorupZhangCWCubic64Init(ThorupZhangCWCubic64_t * k) {
    for(int i = 0; i < 3; ++i) {
        k->A[i]=get64rand();
        k->B[i]=get64rand();
        k->C[i]=get64rand();
        k->D[i]=get64rand();
    }
}


/* cW trick for 64-bit key x with prime 2ˆ89-1 */
__attribute__((always_inline))
inline uint64_t ThorupZhangCWLinear64(uint64_t x, const ThorupZhangCWLinear64_t * k) {
    INT96 r;
    MultAddPrime89(r,x,k->A,k->B);
    return Mod64Prime89(r);
}


/* cW trick for 64-bit key x with prime 2ˆ89-1 */
__attribute__((always_inline))
inline uint64_t ThorupZhangCWQuadratic64(uint64_t x, const ThorupZhangCWQuadratic64_t * k) {
    INT96 r;
    MultAddPrime89(r,x,k->A,k->B);
    MultAddPrime89(r,x,r,k->C);
    return Mod64Prime89(r);
}


/* cW trick for 64-bit key x with prime 2ˆ89-1 */
__attribute__((always_inline))
inline uint64_t ThorupZhangCWCubic64(uint64_t x, const ThorupZhangCWCubic64_t * k) {
    INT96 r;
    MultAddPrime89(r,x,k->A,k->B);
    MultAddPrime89(r,x,r,k->C);
    MultAddPrime89(r,x,r,k->D);
    return Mod64Prime89(r);
}



#endif
