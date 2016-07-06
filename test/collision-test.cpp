#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;

#include "../benchmarks/hashpack.h"



template <typename Pack>
void basictest(const uint32_t coverage, const uint32_t nbr_keys, const uint32_t mindistinct, int nbr_trials, bool * buggy ) {
    std::cout << "testing " << string(Pack::NAME) << std::endl;
    srand(0);
    uint8_t *  bitset = new uint8_t[coverage/8];
    size_t mincount = coverage;
    typename Pack::Randomness * randomness = new typename Pack::Randomness();
    if(randomness == NULL) std::cerr << "Failure to allocate" << std::endl;

    for(int trial = 0 ; trial < nbr_trials; ++trial) {
        Pack::InitRandomness(randomness);

        memset(bitset,0, coverage/8);
        uint32_t oddmultiplier = 555555;
        for(uint32_t k = 0 ; k < nbr_keys; ++k)  {
            typename Pack::Word result = Pack::HashFunction(k * oddmultiplier,randomness);
            bitset[(result % coverage)/ 8] |= (1 << (result % 8));
        }
        size_t count = 0;
        long long * lp = (long long *) bitset;
        for (size_t k = 0; k < (coverage / 8) / sizeof(long long); ++k) { // not efficient
            count += __builtin_popcountll(lp[k]);
        }
        if( count < mincount ) {
          mincount = count;
        }

    }
    delete randomness;
    delete bitset;
    if( mincount < mindistinct) {
      std::cout << string(Pack::NAME) << " got a distinct count of "<< mincount << " hash values in  [0, "<< coverage <<" ) out of " << nbr_keys << " keys "<< std::endl;
      *buggy = true;
    }

}



int main() {

    const uint32_t coverage = 1 << 16;
    const uint32_t nbr_keys = 1 << 10;
    const uint32_t mindistinct = 1000;// out of 1024 values, we expect to hit 1000 distinct 16-bit values
    const int nbr_trials = 1000;
    bool buggy = false;
    basictest<Zobrist64Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);
    basictest<WZobrist64Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);
    basictest<ZobristTranspose64Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);
    basictest<MultiplyShift64Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);
    basictest<ClLinear64Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);
    basictest<ClQuadratic64Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);
    basictest<ClFastQuadratic64Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);
    basictest<ClFast2Quadratic64Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);
    basictest<ClCubic64Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);
    basictest<ClQuartic64Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);
    basictest<ThorupZhangCWLinear64Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);
    basictest<ThorupZhangCWQuadratic64Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);
    basictest<ThorupZhangCWCubic64Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);

    basictest<Zobrist32Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);
    basictest<WZobrist32Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);
    basictest<MultiplyShift32Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);
    basictest<CWQuad32Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);
    basictest<ThorupZhangCWLinear32Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);
    basictest<ThorupZhangCWQuadratic32Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);
    basictest<ThorupZhangCWCubic32Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);

    basictest<ThorupZhang64Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);
    basictest<ThorupZhang32Pack>(coverage, nbr_keys, mindistinct, nbr_trials, &buggy);
    if(buggy) {
      std::cout << "Potential problem detected with some hash families." << std::endl;
    } else {
      std::cout << "Code ok." << std::endl;

    }

}
