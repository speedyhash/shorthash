/**
We hash n 64-bit keys down to integers in [0,n).
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>
#include <algorithm>

#include <sys/resource.h>

#include "hashpack.h"

using namespace std;

struct Worker {
    template <typename Pack>
    static inline void Go(const std::vector<uint64_t> &keys, uint64_t N,
                          const int repeat) {
        std::cout << "testing " << setw(20) << string(Pack::NAME) << " ";
        std::cout.flush();
        std::cout << "hashing  " << keys.size() << " values to  " << N
                  << " buckets, ";

        double max_search_time = 0;
        double min_search_time = numeric_limits<double>::max();
        double total_search_time = 0;

        for (int r = 0; r < repeat; ++r) {
            vector<size_t> buckets(N, 0);
            Pack p;
            double search_time = 0;

            for (const auto key : keys) {
                const auto location = p(key) % keys.size();
                ++buckets[location];
                search_time += buckets[location];
            }

            max_search_time = std::max(search_time, max_search_time);
            min_search_time = std::min(search_time, min_search_time);
            total_search_time += search_time;
        }

        std::cout << "average search time, max, min, avg =   " << setw(10)
                  << max_search_time / keys.size() << setw(10)
                  << min_search_time / keys.size() << setw(10)
                  << total_search_time / (repeat * keys.size()) << std::endl;
    }

    static inline void Stop() {}
};

void demofixed(const uint64_t howmany) {
    srand(0);
    const float repeat = 10000;
    std::vector<uint64_t>  keys;
    for(uint64_t i = 1; i <= howmany; ++i) {
        keys.push_back(i + UINT64_C(555555555));
    }

    uint64_t N = keys.size();
    std::cout << "We repeat with " << repeat << " different hash functions, using the same sequential keys." << std::endl;

    ForEachT<Cyclic64Pack, Zobrist64Pack, MultiplyShift64Pack, ClLinear64Pack,
             ThorupZhangCWCubic64Pack>::template Go<Worker>(keys, N, repeat);

    std::cout << std::endl;

}


int main() {
    vector<uint64_t> sizes{1000,          4000,          1 << 10,
                           1 << 12,       (1 << 10) - 1, (1 << 12) - 1,
                           (1 << 10) + 1, (1 << 12) + 1};
    sort(sizes.begin(), sizes.end());
    for (const auto size : sizes) {
        demofixed(size);
    }
}
