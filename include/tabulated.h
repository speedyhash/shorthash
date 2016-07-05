#ifndef SHORTHASH_TABULATED_H
#define SHORTHASH_TABULATED_H

#include <stdint.h>
#include <limits.h>
#include "util.h"

typedef struct zobrist_s {
    uint64_t hashtab[sizeof(uint64_t)][1 << CHAR_BIT];
} zobrist_t;

void zobrist_init(zobrist_t *k) {
    for (uint32_t i = 0; i < sizeof(uint64_t); i++) {
        for (uint32_t j = 0; j < (1 << CHAR_BIT); j++) {
            k->hashtab[i][j] = get64rand();
        }
    }
}

uint64_t zobrist(uint64_t val, const zobrist_t *k) {
    uint64_t h = 0;
    const unsigned char *s = (const unsigned char *)&val;
    h ^= k->hashtab[0][s[0]];
    h ^= k->hashtab[1][s[1]];
    h ^= k->hashtab[2][s[2]];
    h ^= k->hashtab[3][s[3]];
    h ^= k->hashtab[4][s[4]];
    h ^= k->hashtab[5][s[5]];
    h ^= k->hashtab[6][s[6]];
    h ^= k->hashtab[7][s[7]];
    return h;
}


// "wide" zobrist
typedef struct wzobrist_s {
    uint64_t hashtab[sizeof(uint64_t)/2][1 << 16];
} wzobrist_t;

void wzobrist_init(wzobrist_t *k) {
    for (uint32_t i = 0; i < sizeof(uint64_t)/2; i++) {
        for (uint32_t j = 0; j < (1 << 16); j++) {
            k->hashtab[i][j] = get64rand();
        }
    }
}

uint64_t wzobrist(uint64_t val, const wzobrist_t *k) {
    uint64_t h = 0;
    const uint16_t *s = (const uint16_t *)&val;
    h ^= k->hashtab[0][s[0]];
    h ^= k->hashtab[1][s[1]];
    h ^= k->hashtab[2][s[2]];
    h ^= k->hashtab[3][s[3]];
    return h;
}

// Flat tabulation hashing, in which the randomness data is stored in a
// one-dimensional array, rather than a two-dimensional one
typedef struct zobrist_flat_s {
    uint64_t hashtab[sizeof(uint64_t) << CHAR_BIT];
} zobrist_flat_t;

void zobrist_flat_init(zobrist_flat_t *k) {
  for (uint32_t i = 0; i < (sizeof(uint64_t) << CHAR_BIT); i++) {
      k->hashtab[i] = get64rand();
  }
}

uint64_t zobrist_flat(uint64_t val, const zobrist_flat_t *k) {
    uint64_t h = 0;
    const unsigned char *s = (const unsigned char *)&val;
    h ^= k->hashtab[s[0]];
    h ^= k->hashtab[(1 << CHAR_BIT) + s[1]];
    h ^= k->hashtab[(2 << CHAR_BIT) + s[2]];
    h ^= k->hashtab[(3 << CHAR_BIT) + s[3]];
    h ^= k->hashtab[(4 << CHAR_BIT) + s[4]];
    h ^= k->hashtab[(5 << CHAR_BIT) + s[5]];
    h ^= k->hashtab[(6 << CHAR_BIT) + s[6]];
    h ^= k->hashtab[(7 << CHAR_BIT) + s[7]];
    return h;
}

// The same as zobrist_flat, but treats the array as if the two
// dimensions were flipped in their order.
uint64_t zobrist_flat_transpose(uint64_t val, const zobrist_flat_t *k) {
    uint64_t h = 0;
    const unsigned char *s = (const unsigned char *)&val;
    h ^= k->hashtab[s[0]];
    h ^= k->hashtab[(s[1] * CHAR_BIT) + 1];
    h ^= k->hashtab[(s[2] * CHAR_BIT) + 2];
    h ^= k->hashtab[(s[3] * CHAR_BIT) + 3];
    h ^= k->hashtab[(s[4] * CHAR_BIT) + 4];
    h ^= k->hashtab[(s[5] * CHAR_BIT) + 5];
    h ^= k->hashtab[(s[6] * CHAR_BIT) + 6];
    h ^= k->hashtab[(s[7] * CHAR_BIT) + 7];
    return h;
}

typedef struct zobrist32_s {
    uint32_t hashtab[sizeof(uint32_t)][1 << CHAR_BIT];
} zobrist32_t;

void zobrist32_init(zobrist32_t *k) {
    for (uint32_t i = 0; i < sizeof(uint32_t); i++) {
        for (uint32_t j = 0; j < (1 << CHAR_BIT); j++) {
            k->hashtab[i][j] = get32rand();
        }
    }
}

uint32_t zobrist32(uint32_t val, const zobrist32_t *k) {
    uint32_t h = 0;
    const unsigned char *s = (const unsigned char *)&val;
    h ^= k->hashtab[0][s[0]];
    h ^= k->hashtab[1][s[1]];
    h ^= k->hashtab[2][s[2]];
    h ^= k->hashtab[3][s[3]];
    return h;
}

typedef struct wzobrist32_s {
    uint32_t hashtab[sizeof(uint32_t)/2][1 << 16];
} wzobrist32_t;

void wzobrist32_init(wzobrist32_t *k) {
    for (uint32_t i = 0; i < sizeof(uint32_t)/2; i++) {
        for (uint32_t j = 0; j < (1 << 16); j++) {
            k->hashtab[i][j] = get32rand();
        }
    }
}

uint32_t wzobrist32(uint32_t val, const wzobrist32_t *k) {
    uint32_t h = 0;
    const uint16_t *s = (const uint16_t *)&val;
    h ^= k->hashtab[0][s[0]];
    h ^= k->hashtab[1][s[1]];
    return h;
}

/**
* Rest is from Thorup & Zhang, Tabulation Based 4-Universal Hashing with Applications to
Second Moment Estimation

as well as

Appendix for
Tabulation Based 4-Universal Hashing with Applications to
Second Moment Estimation
*/


typedef struct thorupzhang_s {
    uint64_t basetab[4][1 << 16][2];
    uint64_t hashtab[2][1 << 21];
    uint64_t finalhashtab[1 << 22];
} thorupzhang_t;


// Thorup and Zhang describe how this might be constructed
// but they do not provide software or pseudocode. We
// use random initialization which is just as a good for performance testing
void thorupzhang_init(thorupzhang_t *k) {
    for (uint32_t i = 0; i < 4; i++) {
        for (uint32_t j = 0; j < (1 << 16); j++) {
            k->basetab[i][j][0] = get64rand();
            k->basetab[i][j][1] = get64rand();
        }
    }
    for (uint32_t i = 0; i < 2; i++) {
        for (uint32_t j = 0; j < (1 << 21); j++) {
            k->hashtab[i][j] = get64rand();
        }
    }
    for (uint32_t j = 0; j < (1 << 22); j++) {
        k->finalhashtab[j] = get64rand();
    }
}



static inline uint64_t compress64(uint64_t i) {
  const uint64_t Mask1 = (((uint64_t)4)<<42) + (((uint64_t)4)<<21) + 4;
  const uint64_t Mask2 = (((uint64_t)65535)<<42) + (((uint64_t)65535)<<21) + 65535;
  const uint64_t Mask3 = (((uint64_t)32)<<42) + (((uint64_t)32)<<21) + 31;
  return Mask1 + (i&Mask2) - ((i>>16)&Mask3);
}

uint64_t thorupzhang(uint64_t val, const thorupzhang_t *k) {
    const uint16_t *s = (const uint16_t *)&val;
    const uint64_t * A0 = k->basetab[0][s[0]];
    const uint64_t * A1 = k->basetab[0][s[1]];
    const uint64_t * A2 = k->basetab[0][s[2]];
    const uint64_t * A3 = k->basetab[0][s[3]];
    uint64_t C = compress64(A0[1]+A1[1]+A2[1]+A3[1]);
    return A0[1] ^ A1[1] ^ A2[1] ^ A3[1]
      ^ k->hashtab[0][C & 2097151]
      ^ k->hashtab[1][(C >> 21) & 2097151]
      ^ k->finalhashtab[C >> 42];
}




typedef struct thorupzhang32_s {
    uint32_t hashtab[3][1 << 16];
} thorupzhang32_t;

void thorupzhang32_init(thorupzhang32_t *k) {
    for (uint32_t i = 0; i < 3; i++) {
        for (uint32_t j = 0; j < (1 << 16); j++) {
            k->hashtab[i][j] = get64rand();
        }
    }
}

static inline uint32_t compress32(uint32_t i) {
  return 2 - (i >> 16) + (i & 0xFFFF);
}

uint32_t thorupzhang32(uint32_t val, const thorupzhang32_t *k) {
    uint32_t h = 0;
    uint32_t lowbits = val & 0xFFFF;
    uint32_t highbits = val >> 16;
    h ^= k->hashtab[0][lowbits];
    h ^= k->hashtab[1][highbits];
    h ^= k->hashtab[2][compress32(lowbits + highbits)];
    return h;
}



#endif
