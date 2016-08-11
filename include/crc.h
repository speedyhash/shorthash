#ifndef CRC_H
#define CRC_H

#include <nmmintrin.h> // x86 intrinsics


typedef struct {
  uint32_t crcseed;
} CRCRandomness;

void CRCInit(CRCRandomness *x) {
  x->crcseed = get32rand();
}

// this is CRC32, so result is actually a 32-bit value
uint64_t CRC32_64(uint64_t in, const CRCRandomness *rand) {
    return _mm_crc32_u64(rand->crcseed,in);
}

__attribute__((always_inline))
inline uint32_t CRC32(uint32_t in, const CRCRandomness *rand) {
    return _mm_crc32_u32(rand->crcseed,in);
}

#endif
