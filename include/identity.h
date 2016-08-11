#ifndef IDENTITY_H
#define IDENTITY_H

typedef struct {
} identity_t;

void identity_init(identity_t *key) {
}


uint64_t identity64(uint64_t h, const identity_t * key) {
  (void) key;
  return h;
}

#endif
