#ifndef IDENTITY_H
#define IDENTITY_H

typedef struct { char cpp_compatibility;
} identity_t;

void identity_init(identity_t *key) {
  (void) key;
}

__attribute__((always_inline))
inline uint64_t identity64(uint64_t h, const identity_t * key) {
  (void) key;
  return h;
}

#endif
