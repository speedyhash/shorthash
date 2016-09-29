#ifndef MURMUR_H
#define MURMUR_H

/**
* See https://github.com/aappleby/smhasher/wiki/MurmurHash3
*/
typedef struct {
  int shift1;
  int shift2;
  int shift3;
  uint32_t multiplier1;
  uint32_t multiplier2;
} murmur32_t;

void murmur32_init(murmur32_t *key) {
  key->shift1 = 16;
  key->shift2 = 13;
  key->shift3 = 16;
  key->multiplier1 = UINT32_C(0x85ebca6b);
  key->multiplier2 = UINT32_C(0xc2b2ae35);
}

__attribute__((always_inline))
inline uint32_t murmur32(uint32_t h, const murmur32_t * key) {
  h ^= h >> key->shift1;
  h *= key->multiplier1;
  h ^= h >> key->shift2;
  h *= key->multiplier2;
  h ^= h >> key->shift3;
  return h;
}


typedef struct {
  int shift1;
  int shift2;
  int shift3;
  uint64_t multiplier1;
  uint64_t multiplier2;
} murmur64_t;

void murmur64_init(murmur64_t *key) {
  key->shift1 = 33;
  key->shift2 = 33;
  key->shift3 = 33;
  key->multiplier1 = UINT64_C(0xff51afd7ed558ccd);
  key->multiplier2 = UINT64_C(0xc4ceb9fe1a85ec53);
}

__attribute__((always_inline))
inline uint64_t murmur64(uint64_t h, const murmur64_t * key) {
  h ^= h >> key->shift1;
  h *= key->multiplier1;
  h ^= h >> key->shift2;
  h *= key->multiplier2;
  h ^= h >> key->shift3;
  return h;
}

//http://zimbry.blogspot.ca/2011/09/better-bit-mixing-improving-on.html
void staffordmix01_init(murmur64_t *key) {
  key->shift1 = 31;
  key->shift2 = 27;
  key->shift3 = 33;
  key->multiplier1 = UINT64_C(0x7fb5d329728ea185);
  key->multiplier2 = UINT64_C(0x81dadef4bc2dd44d);
}

void xxhash_init(murmur64_t *key) {
  key->shift1 = 33;
  key->shift2 = 29;
  key->shift3 = 32;
  key->multiplier1 = UINT64_C(4029467366897019727);
  key->multiplier2 = UINT64_C(1609587929392839161);
}


/*
* koloboke is commonly used in java (e.g., from fastutil)
*/
typedef struct {
  int shift1;
  int shift2;
  uint64_t multiplier;
} koloboke_t;

void koloboke_init(koloboke_t *key) {
  key->shift1 = 32;
  key->shift2 = 16;
  key->multiplier = UINT64_C(0x9E3779B97F4A7C15);
}
__attribute__((always_inline))
inline uint64_t koloboke64(uint64_t h, const koloboke_t * key) {
  h *= key->multiplier;
  h ^= h >> key->shift1;
  h ^= h >> key->shift2;
  return h;
}

typedef struct {
    // Random multiplier
    uint64_t multiplier;
} random_koloboke_t;

void random_koloboke_init(random_koloboke_t *key) {
    key->multiplier = 1 | get64rand();
}
__attribute__((always_inline)) inline uint64_t
    random_koloboke64(uint64_t h, const random_koloboke_t *key) {
    h *= key->multiplier;
    h ^= h >> 32;
    h ^= h >> 16;
    return h;
}

typedef struct {
    // Random multiplier, only one shift
    uint64_t multiplier;
} random_weak_koloboke_t;

void random_weak_koloboke_init(random_weak_koloboke_t *key) {
    key->multiplier = 1 | get64rand();
}
__attribute__((always_inline)) inline uint64_t
    random_weak_koloboke64(uint64_t h, const random_weak_koloboke_t *key) {
    h *= key->multiplier;
    h ^= h >> 32;
    return h;
}

#endif
