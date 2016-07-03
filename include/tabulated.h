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

#endif
