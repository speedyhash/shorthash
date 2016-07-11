#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;

#include "../benchmarks/hashpack.h"

struct Worker {
    template <typename Pack>
    static inline void Go(const uint32_t coverage, const uint32_t nbr_keys,
                          const uint32_t mindistinct, int nbr_trials,
                          bool *buggy) {
        std::cout << "testing " << string(Pack::NAME) << std::endl;
        srand(0);
        uint8_t *bitset = new uint8_t[coverage / 8];
        size_t mincount = coverage;
        typename Pack::Randomness *randomness = new typename Pack::Randomness();
        if (randomness == NULL)
            std::cerr << "Failure to allocate" << std::endl;

        for (int trial = 0; trial < nbr_trials; ++trial) {
            Pack::InitRandomness(randomness);

            memset(bitset, 0, coverage / 8);
            uint32_t oddmultiplier = 555555;
            for (uint32_t k = 0; k < nbr_keys; ++k) {
                typename Pack::Word result =
                    Pack::HashFunction(k * oddmultiplier, randomness);
                bitset[(result % coverage) / 8] |= (1 << (result % 8));
            }
            size_t count = 0;
            long long *lp = (long long *)bitset;
            for (size_t k = 0; k < (coverage / 8) / sizeof(long long);
                 ++k) { // not efficient
                count += __builtin_popcountll(lp[k]);
            }
            if (count < mincount) {
                mincount = count;
            }
        }
        delete randomness;
        delete[] bitset;
        if (mincount < mindistinct) {
            std::cout << string(Pack::NAME) << " got a distinct count of "
                      << mincount << " hash values in  [0, " << coverage
                      << " ) out of " << nbr_keys << " keys " << std::endl;
            *buggy = true;
        }
    }
    static inline void Stop() {}
};

int main() {

    const uint32_t coverage = 1 << 16;
    const uint32_t nbr_keys = 1 << 11;
    const uint32_t mindistinct = 95 * nbr_keys / 100;// no more than a 95% collision effect
    const int nbr_trials = 64;
    bool buggy = false;

    ForEachT<Zobrist64Pack, WZobrist64Pack, ZobristTranspose64Pack,
             MultiplyShift64Pack, ClLinear64Pack, ClQuadratic64Pack,
             ClFastQuadratic64Pack, ClFast2Quadratic64Pack, ClCubic64Pack,
             ClQuartic64Pack, ThorupZhangCWLinear64Pack,
             ThorupZhangCWQuadratic64Pack, ThorupZhangCWCubic64Pack,
             Zobrist32Pack, WZobrist32Pack, MultiplyShift32Pack, ClLinear32Pack,
             ClFastQuadratic32Pack, CWQuad32Pack, ThorupZhangCWLinear32Pack,
             ThorupZhangCWQuadratic32Pack, ThorupZhangCWCubic32Pack,
             ThorupZhang64Pack,
             ThorupZhang32Pack>::template Go<Worker>(coverage, nbr_keys,
                                                     mindistinct, nbr_trials,
                                                     &buggy);
    if(buggy) {
      std::cout << "Potential problem detected with some hash families." << std::endl;
    } else {
      std::cout << "Code ok." << std::endl;

    }

}
