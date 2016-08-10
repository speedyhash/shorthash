#ifndef SHORTHASH_UTIL_H
#define SHORTHASH_UTIL_H

#include <stdint.h>
#include <stdlib.h>

typedef unsigned __int128 uint128_t;


static uint32_t get16rand() {
  return rand() >> 7;
}

static uint32_t get32rand() {
    return (((uint32_t)rand() << 0) & 0x0000FFFFul) |
           (((uint32_t)rand() << 16) & 0xFFFF0000ul);
}

static uint64_t get64rand() {
    return (((uint64_t)get32rand()) << 32) | get32rand();
}


static uint128_t get128rand() {
    return (((uint128_t)get64rand()) << 64) | get64rand();
}

#endif
