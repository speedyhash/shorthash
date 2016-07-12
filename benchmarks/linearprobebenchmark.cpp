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
#include <random>
#include <algorithm>
#include <cassert>
#include <cmath>

#include <sys/resource.h>

#include "hashpack.h"

using namespace std;

struct Worker {
    template <typename Pack>
    static inline void Go(std::vector<uint64_t> &keys, uint64_t N,
                          const int repeat) {
        uint32_t howmany = keys.size();
        assert(howmany <= N);
        std::cout << "testing " << setw(20) << string(Pack::NAME) << " ";
        std::cout.flush();
        std::cout << "hashing  " << howmany << " values to  " << N
                  << " buckets, ";
        size_t howmanychars = (N + sizeof(long long)) / 8;
        uint8_t *bitset = new uint8_t[howmanychars];

        uint64_t probing_offset = 0;
        uint64_t max_probing_offset = 0;

        uint64_t probing_offset_square = 0;
        for (int r = 0; r < repeat; ++r) {
            memset(bitset, 0, N / 8);
            Pack p;
            for (uint32_t k = 0; k < howmany; ++k) {
                uint32_t result = p(keys[k]) % N;
                size_t thisoffset = 1; // always one prob
                while ((bitset[result / 8] & (1 << (result % 8))) != 0) {
                    result++;
                    thisoffset++;
                    if (result == N)
                        result = 0;
                }
                assert(thisoffset <= N);
                if (thisoffset > max_probing_offset)
                    max_probing_offset = thisoffset;
                probing_offset += thisoffset;
                probing_offset_square += thisoffset * thisoffset;
                bitset[result / 8] |= (1 << (result % 8));
            }
        }

        double values_stored = repeat * howmany;
        double average_prob = probing_offset / values_stored;

        std::cout << "  average linear probing =   " << setw(10)
                  << average_prob;

        double std_err = sqrt(probing_offset_square / values_stored -
                              average_prob * average_prob);

        std::cout << "  std error  =   " << setw(10) << std_err;
        std::cout << "  max probing  =   " << setw(10) << max_probing_offset;

        delete[] bitset;

        std::cout << std::endl;
    }
    static inline void Stop() {}
};

void demorandom(const uint64_t howmany, const float loadfactor, const int repeat) {
    srand(0);
    std::vector<uint64_t>  keys;
    for(uint64_t i = 1; i <= howmany; ++i) {
        keys.push_back(get64rand());
    }
    std::sort( keys.begin(), keys.end() );
    keys.erase( std::unique( keys.begin(), keys.end() ), keys.end() );

    uint64_t N = ceil(keys.size() / loadfactor);
    std::cout << "We repeat with " << repeat << " different hash functions, using the random same keys." << std::endl;

    ForEachT<Cyclic64Pack, Zobrist64Pack, WZobrist64Pack,  MultiplyShift64Pack, ClLinear64Pack, ClQuadratic64Pack, ClCubic64Pack, ClQuartic64Pack,
             ThorupZhangCWCubic64Pack>::template Go<Worker>(keys, N, repeat);

    std::cout << std::endl;

}

void demofixed(const uint64_t howmany, const float loadfactor, const int repeat) {
    srand(0);
    std::vector<uint64_t>  keys;
    for(uint64_t i = 1; i <= howmany; ++i) {
        keys.push_back(i +  UINT64_C(5555555555));
    }
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(keys.begin(), keys.end(),g);


    uint64_t N = ceil(keys.size() / loadfactor);
    std::cout << "We repeat with " << repeat << " different hash functions, using the same sequential keys." << std::endl;


    ForEachT<Cyclic64Pack, Zobrist64Pack, WZobrist64Pack, MultiplyShift64Pack, ClLinear64Pack, ClQuadratic64Pack, ClCubic64Pack, ClQuartic64Pack,
             ThorupZhangCWCubic64Pack>::template Go<Worker>(keys, N, repeat);

    std::cout << std::endl;

}


int main() {
    float loadfactor = 0.95;
    std::cout << "load factor set at " << loadfactor <<  std::endl;
    const int repeat = 1000;

    demofixed(1000, loadfactor, repeat);
    demofixed(2000, loadfactor, repeat);
    demofixed(64000, loadfactor, repeat);
    demofixed(120000, loadfactor, repeat);

    std::cout << "=======" << std::endl;

    demorandom(1000, loadfactor, repeat);
    demorandom(2000, loadfactor, repeat);
    demorandom(64000, loadfactor, repeat);
    demorandom(120000, loadfactor, repeat);


}
