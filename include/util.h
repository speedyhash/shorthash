#ifndef SHORTHASH_UTIL_H
#define SHORTHASH_UTIL_H

#include <stdint.h>
#include <stdlib.h>

typedef unsigned __int128 uint128_t;

static uint64_t get64rand() {
    return (((uint64_t)rand() << 0) & 0x000000000000FFFFull) |
           (((uint64_t)rand() << 16) & 0x00000000FFFF0000ull) |
           (((uint64_t)rand() << 32) & 0x0000FFFF00000000ull) |
           (((uint64_t)rand() << 48) & 0xFFFF000000000000ull);
}

static uint32_t get32rand() {
    return (((uint32_t)rand() << 0) & 0x0000FFFFul) |
           (((uint32_t)rand() << 16) & 0xFFFF0000ul);
}

static uint128_t get128rand() {
    return (((uint128_t)get64rand()) << 64) | get64rand();
}

#endif
