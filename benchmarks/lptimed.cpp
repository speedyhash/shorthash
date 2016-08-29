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
#include "simple-hashmap.h"
#include "timers.hpp"

using namespace std;

std::random_device rd;
std::mt19937 g(rd());

template <typename ht>
void populate(ht &h, vector<uint64_t> &keys) {
    for (size_t i = 0; i < keys.size(); ++i) {
        h.Insert(keys[i]);
    }
}

template <typename ht>
size_t query(ht &h, vector<uint64_t> &keys, size_t howmany, size_t offset) {
    size_t sum = 0;
    for (size_t i = 0; i < howmany; ++i) {
      sum += h.FindPresentWithOffset(keys[i], offset);
    }
    return sum;
}

template <typename ht>
void query_avg_probed_keys(ht &h, vector<uint64_t> &keys, const size_t offset,
                           double *average, double *stderrprob) {
    double sum = 0;
    double sumsquare = 0;
    for (size_t i = 0; i < keys.size(); ++i) {
        double probed = h.FindPresentWithOffset(keys[i], offset);
        sum += probed;
        sumsquare += probed * probed;
    }
    *average = sum / keys.size();
    *stderrprob = sqrt(sumsquare / keys.size() - *average * *average);
}

template <typename ht>
size_t query_max_probed_keys(ht &h, vector<uint64_t> &keys,
                             const size_t offset) {
    size_t mp = 0;
    for (size_t i = 0; i < keys.size(); ++i) {
        size_t tp = h.FindPresentWithOffset(keys[i], offset);
        if (tp > mp) mp = tp;
    }
    return mp;
}
const uint64_t EMPTY = 0xFFFFFFFFFFFFFFFF;

template <bool robinhood>
struct BasicWorker {
    template <typename Pack>
    static inline void Go(std::vector<std::vector<uint64_t> > &keys,
                          const float loadfactor, const int repeat,
                          size_t howmanyqueries, size_t offset) {
        double querycycles = 0;
        double probesperquery = 0;
        double probesperquerystderr = 0;
        double maxprobesperquery = 0;

        size_t howmany = keys[0].size();
        assert(howmanyqueries <= howmany);
        std::cout << setw(20) << string(Pack::NAME) << " ";
        std::cout.flush();
        uint64_t sum = 0;
        uint64_t cycles_start, cycles_end;
        double max_avg_probes = 0;
        for (int r = 0; r < repeat; ++r) {
            HashSet<uint64_t, Pack> hm;
            cycles_start = RDTSC_START();
            populate(hm, keys[r]);
            cycles_end = RDTSC_FINAL();
            assert(hm.size + hm.has_zero == howmany);

            cycles_start = RDTSC_START();
            sum += query(hm, keys[r], howmanyqueries, offset);
            cycles_end = RDTSC_FINAL();
            querycycles += (cycles_end - cycles_start) * 1.0 / howmanyqueries;
            double pk, pkerr;
            query_avg_probed_keys(hm, keys[r], offset, &pk, &pkerr);
            max_avg_probes = std::max<double>(pk, max_avg_probes);
            double mk = query_max_probed_keys(hm, keys[r], offset);
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

#define MYHASHER                                                            \
    Murmur64Pack, Koloboke64Pack, MultiplyShift64Pack, ClLinear64Pack, Toeplitz64Pack, ClQuadratic64Pack, \
        ClFastQuadratic64Pack, ThorupZhangCWQuadratic64Pack, ClCubic64Pack, \
        ThorupZhangCWCubic64Pack, ClQuartic64Pack, Zobrist64Pack, ThorupZhang64Pack

template <bool robinhood = true>
void demorandom(const uint64_t howmany, const float loadfactor,
                const float repeat, const size_t howmanyqueries,
                const size_t offset) {
    srand(0);
    if (robinhood) std::cout << " Robin Hood activated " << std::endl;
    std::vector<std::vector<uint64_t> > allkeys;
    for (size_t r = 0; r < repeat; ++r) {
        std::vector<uint64_t> keys;
        for (uint64_t i = 1; i <= howmany; ++i) {
            uint64_t newkey = get64rand();
            while (newkey == EMPTY) newkey = get64rand();
            keys.push_back(newkey);
        }
        allkeys.push_back(keys);
    }

    std::cout << "populating a hash table with " << howmany
              << " random 64-bit keys and then retrieving them. " << std::endl;
    std::cout << "load factor = " << loadfactor << std::endl;
    std::cout << "number of consecutive queries on same hash table = "
              << howmanyqueries << std::endl;

    std::cout << "We repeat with " << repeat
              << " different hash functions, using the different keys."
              << std::endl;

    ForEachT<MYHASHER>::template Go<BasicWorker<robinhood> >(
        allkeys, loadfactor, repeat, howmanyqueries, offset);
    std::cout << std::endl;
}

int main() {
    const float loadfactor = 0.5;
    const float repeat = 1;
    const int minsize = 1024;
    const int maxsize = 64 * 1024 * 1024;

    for (int size = minsize; size <= maxsize; size *= 4) {
        for (size_t offset = 1; offset < 64; offset *= 2) {
            demorandom<false>(size, loadfactor, repeat, size, offset);
        }
    }
}
