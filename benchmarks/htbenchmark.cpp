#include <cmath>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

#include <sys/resource.h>

#include "hashpack.h"
#include "hashmap.h"
#include "timers.hpp"

using namespace std;
using namespace rigtorp;

std::random_device rd;
std::mt19937 g(rd());


template <typename ht>
void populate(ht & h, vector<uint64_t> & keys) {
    for(size_t i = 0; i < keys.size(); ++i) {
        h.emplace(keys[i], i);
    }
}

template <typename ht>
size_t  query(ht & h, vector<uint64_t> & keys, size_t howmany) {
    size_t sum = 0;
    for(size_t i = 0; i < howmany; ++i) {
        auto it = h.find(keys[i]);
        if(it != h.end()) sum+= it->second;
    }
    return sum;
}

template <typename ht>
void query_avg_probed_keys(ht & h, vector<uint64_t> & keys, double * average, double * stderrprob) {
    double sum = 0;
    double sumsquare = 0;
    for(size_t i = 0; i < keys.size(); ++i) {
        double probed = h.probed_keys(keys[i]);
        sum += probed;
        sumsquare += probed * probed;
    }
    *average = sum  / keys.size();
    *stderrprob = sqrt(sumsquare /  keys.size() - *average * *average );
}

template <typename ht>
size_t query_max_probed_keys(ht & h, vector<uint64_t> & keys) {
    size_t mp = 0;
    for(size_t i = 0; i < keys.size(); ++i) {
        size_t tp = h.probed_keys(keys[i]);
        if(tp > mp) mp = tp;
    }
    return mp;
}
const uint64_t EMPTY = 0xFFFFFFFFFFFFFFFF;

template <bool robinhood>
struct BasicWorker {
    template <typename Pack>
    static inline void Go(std::vector<std::vector<uint64_t> > &keys, const float loadfactor,
                          const int repeat, size_t howmanyqueries) {
        double querycycles = 0;
        double probesperquery = 0;
        double probesperquerystderr = 0;
        double maxprobesperquery = 0;

        size_t howmany = keys[0].size();
        assert(howmanyqueries <= howmany);
        std::cout << setw(20) << string(Pack::NAME) << " ";
        std::cout.flush();
        uint64_t sum = 0;
        vector<HashMap<uint64_t, uint32_t, Pack, robinhood>> collection;
        uint64_t cycles_start, cycles_end;
        for (int r = 0; r < repeat; ++r) {
            HashMap<uint64_t, uint32_t, Pack, robinhood> hm(16, EMPTY, loadfactor);
            cycles_start = RDTSC_START();
            populate(hm, keys[r]);
            cycles_end = RDTSC_FINAL();
            assert(hm.sanity_check());
            collection.push_back(hm);
            assert( hm.size() == howmany);
        }
        double max_avg_probes = 0;
        for (int r = 0; r < repeat; ++r) {
            HashMap<uint64_t, uint32_t, Pack, robinhood> &hm = collection[r];
            cycles_start = RDTSC_START();
            sum += query(hm, keys[r], howmanyqueries);
            cycles_end = RDTSC_FINAL();
            querycycles += (cycles_end - cycles_start) * 1.0 / howmanyqueries;
            double pk, pkerr;
            query_avg_probed_keys(hm, keys[r], &pk, &pkerr);
            max_avg_probes = std::max<double>(pk, max_avg_probes);
            double mk = query_max_probed_keys(hm, keys[r]);
            probesperquery += pk;
            probesperquerystderr += pkerr;
            maxprobesperquery += mk;
        }
        std::cout << "     " << setw(10) << querycycles / repeat
                  << " avg cycles ";
        std::cout << "     " << setw(10) << probesperquery / repeat
                  << " avg probes ";
        std::cout << "     " << setw(10) << probesperquerystderr / repeat
                  << " avg std. error ";
        std::cout << "     " << setw(10) << maxprobesperquery / repeat
                  << " avg max probes ";
        std::cout << "     " << setw(10) << max_avg_probes
                  << " max avg probes ";
        std::cout << " ignore me: " << sum;
        std::cout << std::endl;
    }
    static inline void Stop() {}
};

#define MYHASHER CRC32_64Pack, Murmur64Pack, Stafford64Pack, xxHash64Pack, ClBitMixing64Pack, BitMixing64Pack, Koloboke64Pack, Cyclic64Pack, Zobrist64Pack,  WZobrist64Pack, ClCubic64Pack , ThorupZhangCWCubic64Pack, MultiplyShift64Pack, ClLinear64Pack, Linear64Pack, Toeplitz64Pack, SplitPack<MultiplyShift64Pack, MultiplyShift64Pack,64>

template <bool robinhood = true>
void demorandom(const uint64_t howmany, const float loadfactor,
                const float repeat, const size_t howmanyqueries) {
    srand(0);
    if(robinhood) std::cout << " Robin Hood activated " << std::endl;
    std::vector<std::vector<uint64_t> > allkeys;
    for(size_t r = 0; r < repeat; ++r)  {
      std::vector<uint64_t>  keys;
      for(uint64_t i = 1; i <= howmany; ++i) {
        uint64_t newkey = get64rand();
        while(newkey == EMPTY) newkey = get64rand();
        keys.push_back(newkey);
      }
      allkeys.push_back(keys);
    }

    std::cout << "populating a hash table with " << howmany << " random 64-bit keys and then retrieving them. " << std::endl;
    std::cout << "load factor = " << loadfactor << std::endl;
    std::cout << "number of consecutive queries on same hash table = " << howmanyqueries << std::endl;

    std::cout << "We repeat with " << repeat << " different hash functions, using the different keys." << std::endl;

    ForEachT<MYHASHER>::template Go<BasicWorker<robinhood> >(allkeys,
                                                                 loadfactor,
                                                                 repeat, howmanyqueries);
    std::cout << std::endl;

}

template <bool robinhood = true>
void demofixed(const uint64_t howmany, const uint32_t gap, const float loadfactor, const float repeat, const size_t howmanyqueries) {
    srand(0);
    if(robinhood) std::cout << " Robin Hood activated " << std::endl;
    std::vector<std::vector<uint64_t> > allkeys;
    for(size_t r = 0; r < repeat; ++r)  {
      std::vector<uint64_t>  keys;
      uint64_t init = get128rand();
      for(uint64_t i = 1; i <= howmany; ++i) {
        keys.push_back(i * gap + init);
      }
      std::shuffle(keys.begin(), keys.end(),g);

      allkeys.push_back(keys);
    }

    std::cout << "populating a hash table with " << howmany << " sequential and shuffled 64-bit keys (gap="<<gap<<") and then retrieving them. " << std::endl;
    std::cout << "load factor = " << loadfactor << std::endl;
    std::cout << "number of consecutive queries on same hash table = " << howmanyqueries << std::endl;

    std::cout << "We repeat with " << repeat << " different hash functions, using the different keys." << std::endl;
    ForEachT<MYHASHER>::template Go<BasicWorker<robinhood> >(allkeys, loadfactor,
                                                            repeat,
                                                            howmanyqueries);
    std::cout << std::endl;

}

uint64_t reversebits(uint64_t v) {
  uint64_t r = v; // r will be reversed bits of v; first get LSB of v
  int s = sizeof(v) * CHAR_BIT - 1; // extra shift needed at end
 for (v >>= 1; v; v >>= 1) {
  r <<= 1;
  r |= v & 1;
  s--;
 }
 r <<= s; // shift when v's highest bits are zero
 return r;
}

template <bool robinhood = true>
void demofillfromtop(const uint64_t howmany, const float loadfactor,  uint32_t repeat,const size_t howmanyqueries) {
    srand(0);
    if(robinhood) std::cout << " Robin Hood activated " << std::endl;
    std::vector<std::vector<uint64_t> > allkeys;
    for(size_t r = 0; r < repeat; ++r)  {
      std::vector<uint64_t>  keys;
      for(uint64_t i = 0; i <= howmany; ++i) {
        keys.push_back(reversebits(i));
      }
      std::shuffle(keys.begin(), keys.end(),g);
      allkeys.push_back(keys);
    }

    std::cout << "populating a hash table with filled from the top and then retrieving them. " << std::endl;
    std::cout << "load factor = " << loadfactor << std::endl;
    std::cout << "number of consecutive queries on same hash table = " << howmanyqueries << std::endl;

    std::cout << "We repeat with " << repeat << " different hash functions, using the different keys." << std::endl;
    ForEachT<MYHASHER>::template Go<BasicWorker<robinhood> >(allkeys, loadfactor,
                                                            repeat,
                                                            howmanyqueries);
    std::cout << std::endl;

}

int main() {
    const float loadfactor = 0.9;
    const float repeat = 100;
    const size_t howmanyqueries = 10;
    const int maxsize = 8000;//128000;
    const int minsize = 8000;//128000;

    std::cout << "=======" << std::endl;

    for(int size = minsize; size <= maxsize; size*= 2)
      demofixed<true>(size, 1, loadfactor,  repeat, howmanyqueries);
    for(int size = minsize; size <= maxsize; size*= 2)
      demofixed<false>(size, 1, loadfactor,  repeat, howmanyqueries);

    std::cout << "=======" << std::endl;

    for(int size = minsize; size <=maxsize; size*= 2)
      demofillfromtop<true>(size, loadfactor,  repeat, howmanyqueries);
    for(int size = minsize; size <=maxsize; size*= 2)
      demofillfromtop<false>(size, loadfactor,  repeat, howmanyqueries);

    std::cout << "=======" << std::endl;

    for(int size = minsize; size <= maxsize; size*= 2)
      demofixed<true>(size, 1<<16, loadfactor,  repeat, howmanyqueries);
    for(int size = minsize; size <= maxsize; size*= 2)
      demofixed<false>(size, 1<<16, loadfactor,  repeat, howmanyqueries);
    std::cout << "=======" << std::endl;

    for(int size = minsize; size <= maxsize; size*= 2)
      demofixed<true>(size, 1<<31, loadfactor,  repeat, howmanyqueries);
    for(int size = minsize; size <= maxsize; size*= 2)
      demofixed<false>(size, 1<<31, loadfactor,  repeat, howmanyqueries);

    std::cout << "=======" << std::endl;

    for(int size = minsize; size <= maxsize; size*= 2)
      demorandom<true>(size, loadfactor,  repeat, howmanyqueries);
    for(int size = minsize; size <= maxsize; size*= 2)
      demorandom<false>(size, loadfactor,  repeat, howmanyqueries);

}
