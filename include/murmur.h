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


uint32_t murmur32(uint32_t h, const murmur32_t * key) {
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

uint64_t murmur64(uint64_t h, const murmur64_t * key) {
  h ^= h >> key->shift1;
  h *= key->multiplier1;
  h ^= h >> key->shift2;
  h *= key->multiplier2;
  h ^= h >> key->shift3;
  return h;
}


#endif
