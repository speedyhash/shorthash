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

#include "buckets.hpp"
#include "hashpack.h"

using namespace std;

struct Worker {
    template <typename Pack>
    static inline void Go(Reducer64 mod, const std::vector<uint64_t> &keys,
                          uint64_t N, const int repeat) {
        std::cout << "testing " << setw(20) << string(Pack::NAME) << " ";
        std::cout.flush();
        std::cout << "hashing  " << keys.size() << " values to  " << N
                  << " buckets, ";

        double max_search_time = 0;
        double min_search_time = numeric_limits<double>::max();
        double total_search_time = 0;

        for (int r = 0; r < repeat; ++r) {
            Pack p;
            const double search_time = SearchTime(p, mod, N, keys);

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

template<typename... Packs>
void demofixed(Reducer64 mod, const uint64_t howmany) {
    srand(0);
    const float repeat = 10000;
    std::vector<uint64_t>  keys;
    for(uint64_t i = 1; i <= howmany; ++i) {
        keys.push_back(i + UINT64_C(555555555));
    }

    uint64_t N = keys.size();
    std::cout << "We repeat with " << repeat << " different hash functions, using the same sequential keys." << std::endl;

    ForEachT<Packs...>::template Go<Worker>(mod, keys, N, repeat);

    std::cout << std::endl;

}

int main() {
    vector<uint64_t> sizes{1000,          4000,          1 << 10,
                           1 << 12,       (1 << 10) - 1, (1 << 12) - 1,
                           (1 << 10) + 1, (1 << 12) + 1};
    sort(sizes.begin(), sizes.end());
    vector<Reducer64> mods {Modulo64, FastModulo64};
    for (const auto size : sizes) {
        for (const auto mod : mods) {
            cout << "With " << ((FastModulo64 == mod) ? "Fast" : "Slow")
                 << " Mod" << endl;
            demofixed<Cyclic64Pack, Zobrist64Pack, MultiplyShift64Pack,
                      UnivMultiplyShift64Pack, ClLinear64Pack,
                      ThorupZhangCWCubic64Pack, MultiplyTwice64Pack>(mod, size);
        }
    }
}
