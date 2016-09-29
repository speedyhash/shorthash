#ifndef ODDMULTIPLY_H
#define ODDMULTIPLY_H

#include "util.h"

typedef struct { uint64_t multiplier;
} oddmultiply64_t;

void oddmultiply64_init(oddmultiply64_t *key) {
  key->multiplier = get64rand() | 1; // random odd numbers
}

// we reversed because our hashing algos will use the lower bits
uint64_t reversed_oddmultiply64(uint64_t h, const oddmultiply64_t * key) {
  h *= key->multiplier;
  return kbitreverse64(h);
}

#endif
