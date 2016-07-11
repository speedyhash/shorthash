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
    static inline void Go(std::vector<uint64_t> &keys, uint64_t N,
                          const int repeat) {
        uint32_t howmany = keys.size();
        std::cout << "testing " << setw(20) << string(Pack::NAME) << " ";
        std::cout.flush();
        std::cout << "hashing  " << howmany << " values to  " << N
                  << " buckets, ";
        uint64_t sum = 0;
        size_t howmanychars = (N + sizeof(long long)) / 8;
        uint8_t *bitset = new uint8_t[howmanychars];

        for (int r = 0; r < repeat; ++r) {
            memset(bitset, 0, N / 8);

            Pack p;

            for (uint32_t k = 0; k < howmany; ++k) {
                const uint32_t result = p(keys[k]);
                bitset[(result % N) / 8] |= (1 << (result % 8));
            }

            size_t count = 0;
            long long *lp = (long long *)bitset;
            for (size_t k = 0; k < howmanychars / sizeof(long long);
                 ++k) { // not efficient
                count += __builtin_popcountll(lp[k]);
            }
            sum += count;
        }
        delete[] bitset;

        double buckets_used = sum;
        double values_stored = repeat * howmany;

        std::cout << "  average bucket size =   " << setw(10)
                  << values_stored / buckets_used;
        std::cout << std::endl;
    }
    static inline void Stop() {}
};

void demorandom(const uint64_t howmany) {
    srand(0);
    const float repeat = 1000;
    std::vector<uint64_t>  keys;
    for(uint64_t i = 1; i <= howmany; ++i) {
        keys.push_back(get64rand());
    }
    std::sort( keys.begin(), keys.end() );
    keys.erase( std::unique( keys.begin(), keys.end() ), keys.end() );

    uint64_t N = keys.size();
    std::cout << "We repeat with " << repeat << " different hash functions, using the random same keys." << std::endl;

    ForEachT<Zobrist64Pack, MultiplyShift64Pack, ClLinear64Pack,
             ThorupZhangCWCubic64Pack>::template Go<Worker>(keys, N, repeat);

    std::cout << std::endl;

}

void demofixed(const uint64_t howmany) {
    srand(0);
    const float repeat = 1000;
    std::vector<uint64_t>  keys;
    for(uint64_t i = 1; i <= howmany; ++i) {
        keys.push_back(i +  UINT64_C(5555555555));
    }
    std::sort( keys.begin(), keys.end() );
    keys.erase( std::unique( keys.begin(), keys.end() ), keys.end() );

    uint64_t N = keys.size();
    std::cout << "We repeat with " << repeat << " different hash functions, using the same sequential keys." << std::endl;


    ForEachT<Zobrist64Pack, MultiplyShift64Pack, ClLinear64Pack,
             ThorupZhangCWCubic64Pack>::template Go<Worker>(keys, N, repeat);

    std::cout << std::endl;

}


int main() {
    demorandom(1000);
    demorandom(2000);
    demorandom(64000);
    demorandom(120000);

    demofixed(1000);
    demofixed(2000);
    demofixed(64000);
    demofixed(120000);

}
