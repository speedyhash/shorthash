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
#include <cassert>
#include <cmath>

#include <sys/resource.h>

#include "hashpack.h"

using namespace std;





template <typename Pack>
void basic(std::vector<uint64_t> & keys, uint64_t N  , const int repeat) {
    uint32_t howmany = keys.size();
    assert(howmany <= N);
    std::cout << "testing "  << setw(20) << string(Pack::NAME) << " ";
    std::cout.flush();
    std::cout << "hashing  "  << howmany << " values to  "<< N << " buckets, ";
    size_t howmanychars = (N + sizeof(long long) ) / 8;
    uint8_t *  bitset = new uint8_t[howmanychars];

    uint64_t probing_offset = 0;
    for(int r = 0 ; r < repeat; ++r) {
        memset(bitset,0, N/8);
        Pack p;
        for(uint32_t k = 0 ; k < howmany; ++k)  {
             uint32_t result = p(keys[k]) % N;
            size_t thisoffset = 0;
            while((bitset[result/ 8] & (1 << (result % 8))) != 0 ) {
              result ++;
              thisoffset++;
              if (result == N) result = 0;
            }
            probing_offset += thisoffset;
            bitset[result / 8] |= (1 << (result % 8));
        }
    }
    delete[] bitset;

    double values_stored = repeat * howmany;

     std::cout << "  average linear probing =   " << setw(10) << probing_offset / values_stored ;

    std::cout << std::endl;

}

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


    basic<Zobrist64Pack>(keys, N, repeat);

    basic<MultiplyShift64Pack>(keys, N, repeat);
    basic<ClLinear64Pack>(keys, N, repeat);
    basic<ThorupZhangCWCubic64Pack>(keys, N, repeat);

    std::cout << std::endl;

}

void demofixed(const uint64_t howmany, const float loadfactor, const int repeat) {
    srand(0);
    std::vector<uint64_t>  keys;
    for(uint64_t i = 1; i <= howmany; ++i) {
        keys.push_back(i +  UINT64_C(5555555555));
    }
    std::sort( keys.begin(), keys.end() );
    keys.erase( std::unique( keys.begin(), keys.end() ), keys.end() );

    uint64_t N = ceil(keys.size() / loadfactor);
    std::cout << "We repeat with " << repeat << " different hash functions, using the same sequential keys." << std::endl;


    basic<Zobrist64Pack>(keys, N, repeat);

    basic<MultiplyShift64Pack>(keys, N, repeat);
    basic<ClLinear64Pack>(keys, N, repeat);
    basic<ThorupZhangCWCubic64Pack>(keys, N, repeat);

    std::cout << std::endl;

}


int main() {
    float loadfactor = 0.98;
    std::cout << "load factor set at " << loadfactor <<  std::endl;
    const int repeat = 1000;
    demorandom(1000, loadfactor, repeat);
    demorandom(2000, loadfactor, repeat);
    demorandom(4000, loadfactor, repeat);
    demorandom(8000, loadfactor, repeat);
    demorandom(16000, loadfactor, repeat);
    demorandom(32000, loadfactor, repeat);
    demorandom(64000, loadfactor, repeat);
    demorandom(128000, loadfactor, repeat);

    demofixed(1000, loadfactor, repeat);
    demofixed(2000, loadfactor, repeat);
    demofixed(64000, loadfactor, repeat);
    demofixed(120000, loadfactor, repeat);

}
