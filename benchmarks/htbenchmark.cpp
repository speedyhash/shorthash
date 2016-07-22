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

struct BasicWorker {
    template <typename Pack>
    static inline void Go(std::vector<uint64_t> &keys, const float loadfactor,
                          const int repeat, size_t howmanyqueries) {
        double querycycles = 0;
        double probesperquery = 0;
        double probesperquerystderr = 0;
        double maxprobesperquery = 0;

        size_t howmany = keys.size();
        assert(howmanyqueries <= howmany);
        std::cout << "testing " << setw(20) << string(Pack::NAME) << " ";
        std::cout.flush();
        uint64_t sum = 0;
        vector<HashMap<uint64_t, uint32_t, Pack>> collection;
        uint64_t cycles_start, cycles_end;
        for (int r = 0; r < repeat; ++r) {
            HashMap<uint64_t, uint32_t, Pack> hm(16, EMPTY, loadfactor);
            cycles_start = RDTSC_START();
            populate(hm, keys);
            cycles_end = RDTSC_FINAL();
            collection.push_back(hm);
        }
        double max_avg_probes = 0;
        for (int r = 0; r < repeat; ++r) {
            HashMap<uint64_t, uint32_t, Pack> &hm = collection[r];
            cycles_start = RDTSC_START();
            sum += query(hm, keys, howmanyqueries);
            cycles_end = RDTSC_FINAL();
            querycycles += (cycles_end - cycles_start) * 1.0 / howmanyqueries;
            double pk, pkerr;
            query_avg_probed_keys(hm, keys, &pk, &pkerr);
            max_avg_probes = std::max<double>(pk, max_avg_probes);
            double mk = query_max_probed_keys(hm, keys);
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

void demorandom(const uint64_t howmany, const float loadfactor,
                const float repeat, const size_t howmanyqueries) {
    srand(0);
    std::vector<uint64_t>  keys;
    for(uint64_t i = 1; i <= howmany; ++i) {
      uint64_t newkey = get64rand();
      while(newkey == EMPTY) newkey = get64rand();
      keys.push_back(newkey);
    }
    std::cout << "populating a hash table with " << howmany << " random 64-bit keys and then retrieving them. " << std::endl;
    std::cout << "load factor = " << loadfactor << std::endl;
    std::cout << "number of consecutive queries on same hash table = " << howmanyqueries << std::endl;

    std::cout << "We repeat with " << repeat << " different hash functions, using the same keys." << std::endl;

    ForEachT<ClBitMixing64Pack, Murmur64Pack, CRC32_64Pack, Cyclic64Pack, Zobrist64Pack,  MultiplyShift64Pack, WZobrist64Pack, ClLinear64Pack, ClQuadratic64Pack, ClCubic64Pack, ClQuartic64Pack,
             ThorupZhangCWCubic64Pack>::template Go<BasicWorker>(keys,
                                                                 loadfactor,
                                                                 repeat, howmanyqueries);

    std::cout << std::endl;

}

void demofixed(const uint64_t howmany, const float loadfactor, const float repeat, const size_t howmanyqueries) {
    srand(0);
    std::vector<uint64_t>  keys;
    for(uint64_t i = 1; i <= howmany; ++i) {
        keys.push_back(i + UINT64_C(0xFFFFFFFFF));
    }
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(keys.begin(), keys.end(),g);

    std::cout << "populating a hash table with " << howmany << " sequential and shuffled 64-bit keys and then retrieving them. " << std::endl;
    std::cout << "load factor = " << loadfactor << std::endl;
    std::cout << "number of consecutive queries on same hash table = " << howmanyqueries << std::endl;

    std::cout << "We repeat with " << repeat << " different hash functions, using the same keys." << std::endl;

    ForEachT<
        ClBitMixing64Pack, Murmur64Pack, CRC32_64Pack, Cyclic64Pack, Zobrist64Pack, MultiplyShift64Pack,
        ClLinear64Pack, MultiplyTwice64Pack, ClQuadratic64Pack,
        MultiplyThrice64Pack, ClCubic64Pack, ClQuartic64Pack,
        ThorupZhangCWCubic64Pack>::template Go<BasicWorker>(keys, loadfactor,
                                                            repeat,
                                                            howmanyqueries);

    std::cout << std::endl;

}


int main() {
    const float loadfactor = 0.9;
    const float repeat = 1000;
    const size_t howmanyqueries = 10;

    demofixed(1000, loadfactor, repeat, howmanyqueries);
    demofixed(2000, loadfactor, repeat, howmanyqueries);
    demofixed(64000, loadfactor, repeat, howmanyqueries);
    demofixed(120000, loadfactor, repeat, howmanyqueries);

    std::cout << "=======" << std::endl;

    demorandom(1000, loadfactor, repeat, howmanyqueries);
    demorandom(2000, loadfactor, repeat, howmanyqueries);
    demorandom(64000, loadfactor, repeat, howmanyqueries);
    demorandom(120000, loadfactor, repeat, howmanyqueries);

}
