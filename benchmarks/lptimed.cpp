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
#include "sep-chaining.h"
#include "timers.hpp"
#include "rehashset.h"

using namespace std;

template <typename ht>
void populate(ht &h, const vector<uint64_t> &keys) {
    for (size_t i = 0; i < keys.size(); ++i) {
        h.Insert(keys[i]);
    }
}

template <typename ht>
size_t query(ht &h, const vector<uint64_t> &keys, size_t howmany) {
    size_t sum = 0;
    for (size_t i = 0; i < howmany; ++i) {
      sum += h.FindPresent(keys[i]);
    }
    return sum;
}

template <typename ht>
void query_avg_probed_keys(ht &h, const vector<uint64_t> &keys, double *average,
                           double *stderrprob) {
    double sum = 0;
    double sumsquare = 0;
    for (size_t i = 0; i < keys.size(); ++i) {
        double probed = h.FindPresent(keys[i]);
        sum += probed;
        sumsquare += probed * probed;
    }
    *average = sum / keys.size();
    *stderrprob = sqrt(sumsquare / keys.size() - *average * *average);
}

template <typename ht>
size_t query_max_probed_keys(ht &h, const vector<uint64_t> &keys) {
    size_t mp = 0;
    for (size_t i = 0; i < keys.size(); ++i) {
        size_t tp = h.FindPresent(keys[i]);
        if (tp > mp) mp = tp;
    }
    return mp;
}


struct BasicWorker {
    template <typename Set>
    static inline void Go(const std::vector<uint64_t> &keys, const int repeat,
                          size_t howmanyqueries) {
        double querycycles = 0;
        double probesperquery = 0;
        double probesperquerystderr = 0;
        double maxprobesperquery = 0;
        double constructiontimepervalue = 0;

        assert(howmanyqueries <= keys.size());
        std::cout << setw(20) << string(Set::Name()) << " ";
        std::cout.flush();
        uint64_t sum = 0;
        uint64_t cycles_start, cycles_end;
        double max_avg_probes = 0;
        for (int r = 0; r < repeat; ++r) {
            Set inithm;
            cycles_start = RDTSC_START();
            populate(inithm, keys);
            cycles_end = RDTSC_FINAL();
            assert(inithm.Ndv() == keys.size());
            std::vector<uint64_t> ordered_keys(keys.size());
            size_t howmany = inithm.GetKeys(ordered_keys);
            assert(howmany == keys.size());
            Set hm;
            cycles_start = RDTSC_START();
            populate(hm, keys);
            cycles_end = RDTSC_FINAL();
            constructiontimepervalue += (cycles_end - cycles_start) * 1.0 / keys.size();
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
        std::cout << "     " << setw(10) << constructiontimepervalue / repeat
                  << " avg cycles ";
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
#define MYHASHER HashSet<uint64_t, ClQuartic64Pack>, HashSet<uint64_t, Zobrist64Pack>,\
HashSet<uint64_t, JavaSplit64Pack>, HashSet<uint64_t, Murmur64Pack>,\
HashSet<uint64_t, WZobrist64Pack>

// disabled temporarily SplitHashSet and ReHashSet
/*#define MYHASHER                                                              \
    SepChain<uint64_t, UnivMultiplyShift64Pack>,                              \
        HashSet<uint64_t, ClQuartic64Pack>, HashSet<uint64_t, Zobrist64Pack>, \
        SplitHashSet<uint64_t, UnivMultiplyShift64Pack, Zobrist64Pack, 1>,    \
        SplitHashSet<uint64_t, UnivMultiplyShift64Pack, Zobrist64Pack, 8>,    \
        SplitHashSet<uint64_t, UnivMultiplyShift64Pack, Zobrist64Pack, 32>,   \
        SplitHashSet<uint64_t, UnivMultiplyShift64Pack, Zobrist64Pack, 1024>, \
        SplitHashSet<uint64_t, MultiplyShift64Pack, Zobrist64Pack, 32>,       \
        ReHashSet<uint64_t, UnivMultiplyShift64Pack>
*/
void demorandom(const uint64_t howmany, const float repeat,
                const size_t howmanyqueries) {
    srand(0);
    std::vector<uint64_t> keys;
    for (uint64_t i = 0; i < howmany; ++i) {
        keys.push_back(get64rand());
    }
    std::cout << "displaying construction time per key, time per query, "
    "probes per query, max probe per query and max avg probe " << endl;
    std::cout << howmany << " keys, " << repeat << " hash functions"
              << std::endl;

    ForEachT<MYHASHER>::template Go<BasicWorker>(keys, repeat, howmanyqueries);
    std::cout << std::endl;
}

int main() {
    const float repeat = 100;
    const int minsize = 1024;
    const int maxsize = 64 * 1024 * 1024;

    for (int size = minsize; size <= maxsize; size *= 4) {
        demorandom(size, repeat, size);
    }
}
