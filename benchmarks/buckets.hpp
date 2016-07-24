#ifndef SHORTHASH_BENCHMARKS_BUCKETS_HPP
#define SHORTHASH_BENCHMARKS_BUCKETS_HPP

#include <cstdint>
#include <cstddef>

#include <vector>

typedef uint64_t (*Reducer64)(uint64_t hash, uint64_t modulus);

uint64_t Modulo64(uint64_t hash, uint64_t modulus) { return hash % modulus; }

uint64_t FastModulo64(uint64_t hash, uint64_t modulus) {
    const unsigned __int128 hash128 = hash;
    const unsigned __int128 modulus128 = modulus;
    return (hash128 * modulus128) >> 64;
}

template <typename Pack, typename KeyType>
std::size_t SearchTime(const Pack &p, Reducer64 mod, std::uint64_t N,
                       const std::vector<KeyType> &keys) {
    std::vector<std::size_t> buckets(N, 0);
    std::size_t search_time = 0;

    for (const auto key : keys) {
        const auto location = mod(p(key), keys.size());
        ++buckets[location];
        search_time += buckets[location];
    }
    return search_time;
}

#endif
