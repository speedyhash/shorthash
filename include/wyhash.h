
#ifndef WYHASH_H
#define WYHASH_H


void wyhash_init(uint64_t*) {

}
uint64_t wyhash(uint64_t state, const uint64_t*) {
    __uint128_t tmp = (__uint128_t)(state)*0xa3b195354a39b70dull;
    uint64_t m1 = (tmp >> 64) ^ tmp;
    tmp = (__uint128_t)m1 * 0x1b03738712fad5c9ull;
    return (tmp >> 64) ^ tmp;
}

#endif