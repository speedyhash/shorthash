#ifndef FNV_H
#define FNV_H

/**
* This is FNV-1a
* See https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
*/
typedef struct {
  uint64_t basis;
  uint64_t primemultiplier;
} fnv64_t;

void fnv64_init(fnv64_t *key) {
  key->basis = UINT64_C(0xcbf29ce484222325);
  key->primemultiplier = UINT64_C(0x100000001b3);
}

__attribute__((always_inline))
inline uint64_t fnv64(uint64_t h, const fnv64_t * key) {
  const uint8_t * aschars = (const uint8_t *) & h;
  uint64_t hash = key->basis;
  for(int p = 0; p < 8; p++) {  
    hash ^= aschars[p];
    hash *= key->primemultiplier;
  }
  return hash;
}

#endif
