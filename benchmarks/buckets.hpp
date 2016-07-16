#ifndef SHORTHASH_BENCHMARKS_BUCKETS_HPP
#define SHORTHASH_BENCHMARKS_BUCKETS_HPP

#include <cstdint>
#include <cstddef>

#include <vector>

template <typename Pack, typename KeyType>
std::size_t SearchTime(const Pack &p, std::uint64_t N,
                       const std::vector<KeyType> &keys) {
    std::vector<std::size_t> buckets(N, 0);
    std::size_t search_time = 0;

    for (const auto key : keys) {
        const auto location = p(key) % keys.size();
        ++buckets[location];
        search_time += buckets[location];
    }
    return search_time;
}

#endif
