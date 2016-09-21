#include <cmath>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <time.h>
#include <random>
#include <vector>
#include <unistd.h>

#include <sys/resource.h>
#include "hashpack.h"
#include "hashmap.h"

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
void query_avg_probed_keys(ht & h, vector<uint64_t> & keys, double * average, double * stderrprob, vector<uint32_t> & counts) {
    double sum = 0;
    double sumsquare = 0;
    for(size_t i = 0; i < keys.size(); ++i) {
        double probed = h.probed_keys(keys[i]);
        if(counts.size() <= probed) counts.resize(probed + 1);
        counts[probed] += 1;
        sum += probed;
        sumsquare += probed * probed;
    }
    *average = sum  / keys.size();
    *stderrprob = sqrt(sumsquare /  keys.size() - *average **average );
}

const uint64_t EMPTY = 0xFFFFFFFFFFFFFFFF;

template <bool robinhood>
struct BasicWorker {
    template <typename Pack>
    static inline void Go(std::vector<uint64_t> &keys, const float loadfactor) {
        double probesperquery = 0;
        double probesperquerystderr = 0;
        vector<uint32_t> counts;

        std::cout <<"#" << setw(20) << string(Pack::NAME) << " ";
        std::cout.flush();
        HashMap<uint64_t, uint32_t, Pack, robinhood> hm(16, EMPTY, loadfactor);
        populate(hm, keys);
        size_t bucketcount = hm.bucket_count();
        size_t effectivesize = hm.size();
        assert(effectivesize == keys.size());
        double effectiveload = effectivesize * 1.0 / bucketcount ;
        query_avg_probed_keys(hm, keys, &probesperquery, &probesperquerystderr, counts);
        std::cout << "    effective load: " << setw(10) << effectiveload
                  << ", ";
        std::cout << "     " << setw(10) << probesperquery
                  << " avg probes ";
        std::cout << "     " << setw(10) << probesperquerystderr
                  << " avg std. error ";
        std::cout << std::endl;
        std::cout << effectiveload<< "\t"<<probesperquery<< "\t" << probesperquerystderr  << std::endl;
        size_t mc = 0;
        std::cout << "# next line is histogram of probing distances "<<endl;
        for(size_t i = 0; i < counts.size() ; ++i) {
          std::cout << setw(10) << counts[i] ;
          mc += counts[i];
        }
        assert(mc == keys.size());
        std::cout << std::endl;
    }
};


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





enum {GEOMETRIC=0, FROMTOP=1, RANDOM=2, GRAYCODE = 3};
const char*models[] = {"geometric","fromtop","random", "graycode"};


const char* hashfamilies[] = {"murmur","koloboke","zobrist", "wide-zobrist", "tztabulated", "cllinear", "clquadratic", "clcubic", "cwlinear", "cwquadratic", "cwcubic","multiplyshift", "cyclic", "fnv", "identity"};

void printusage(const char * name) {
    printf("Usage: %s -l [maxloadfactor:0-1] -s [size:>0] -m [model:0-%d] -H [hashfamily:0-%d]\n",name,(int)(sizeof(models)/sizeof(models[0]))-1,(int)(sizeof(hashfamilies)/sizeof(hashfamilies[0]))-1);
    printf("\n");
    for(int i = 0; i < (int)(sizeof(models)/sizeof(models[0])); ++i) {
        printf("model %d is %s \n",i,models[i]);
    }
    for(size_t i = 0; i < sizeof(hashfamilies)/sizeof(hashfamilies[0]); ++i) {
        printf("hashfamily %zu is %s \n",i,hashfamilies[i]);
    }
}


int main(int argc, char **argv) {
    std::cout << "# this program constructs one hash table and reports stats." <<std::endl;
    float loadfactor = -1;
    int size = -1;
    int model = -1;
    int hasher = -1;
    int seed = 0;
    struct timespec currenttime;
    if( clock_gettime( CLOCK_REALTIME, &currenttime) == -1 ) {
      seed = time(NULL);
    } else {
      seed = currenttime.tv_nsec;
    }
    printf("# random seed = %d \n",seed);
    srand (seed);
    int c;
    if(argc == 1) {
        printusage(argv[0]);
        return -1;
    }
    while ((c = getopt(argc, argv, "hl:s:m:H:")) != -1) switch (c) {
        case 'l':
            loadfactor = atof(optarg);
            printf("# using max. load factor of %f \n",loadfactor);
            break;
        case 's':
            size = atoi(optarg);
            printf("# using size of %d \n",size);
            break;
        case 'H':
            hasher = atoi(optarg);
            break;
        case 'm':
            model = atoi(optarg);
            printf("# using model %s \n",models[model]);
            break;

        case 'h':
            printusage(argv[0]);
            return 0;
        default:
            abort();
        }
    if(size <=0) {
        printf("size should be positive.\n");
        printusage(argv[0]);
        return -1;
    }
    if((model <0) ||(model>=(int)(sizeof(models)/sizeof(models[0])))) {
        printf("model value should be between 0 and %d , is %d\n",(int)(sizeof(models)/sizeof(models[0])),model);
        printusage(argv[0]);

        return -1;
    }
    if((loadfactor <=0) ||(loadfactor>1)) {
        printf("loadfactor should be between 0 and 1");
        printusage(argv[0]);
        return -1;
    }
    std::vector<uint64_t>  keys;
    size_t howmany = size;

    if(model == FROMTOP) {
        uint64_t init = get64rand();
        for(uint64_t i = 0; i < howmany; ++i) {
            keys.push_back(reversebits(i + init));
        }
    } else if (model == GEOMETRIC) {
        uint64_t init = get64rand();
        for(uint64_t i = 0; i < howmany; ++i) {
            keys.push_back(i + init);
        }
    } else if (model == GRAYCODE) {
        uint64_t init = get64rand();
        for(uint64_t i = 0; i < howmany; ++i) {
            uint64_t val = i + init;
            val = val ^ ( val >> 1);
            keys.push_back(i + init);
        }
    } else {
        for(uint64_t i = 0; i < howmany; ++i) {
            keys.push_back(get64rand());
        }

    }
    std::shuffle(keys.begin(), keys.end(),g);
    const bool robinhood = true;
    if(robinhood) std::cout << "# using robin-hood hashing to minimize probing distance."<< std::endl;
    switch (hasher) {
    case 0:
        BasicWorker<robinhood>::Go<Murmur64Pack>(keys,loadfactor);
        break;
    case 1:
        BasicWorker<robinhood>::Go<Koloboke64Pack>(keys,loadfactor);
        break;
    case 2:
        BasicWorker<robinhood>::Go<Zobrist64Pack>(keys,loadfactor);
        break;
    case 3:
        BasicWorker<robinhood>::Go<WZobrist64Pack>(keys,loadfactor);
        break;
    case 4:
        BasicWorker<robinhood>::Go<ThorupZhang64Pack>(keys,loadfactor);
        break;
    case 5:
        BasicWorker<robinhood>::Go<ClLinear64Pack>(keys,loadfactor);
        break;
    case 6:
        BasicWorker<robinhood>::Go<ClFastQuadratic64Pack>(keys,loadfactor);
        break;
    case 7:
        BasicWorker<robinhood>::Go<ClCubic64Pack>(keys,loadfactor);
        break;
    case 8:
        BasicWorker<robinhood>::Go<ThorupZhangCWLinear64Pack>(keys,loadfactor);
        break;
    case 9:
        BasicWorker<robinhood>::Go<ThorupZhangCWQuadratic64Pack>(keys,loadfactor);
        break;
    case 10:
        BasicWorker<robinhood>::Go<ThorupZhangCWCubic64Pack>(keys,loadfactor);
        break;
    case 11:
        BasicWorker<robinhood>::Go<MultiplyShift64Pack>(keys,loadfactor);
        break;
    case 12:
        BasicWorker<robinhood>::Go<Cyclic64Pack>(keys,loadfactor);
        break;
    case 13:
        BasicWorker<robinhood>::Go<FNV64Pack>(keys,loadfactor);
        break;
    case 14:
        BasicWorker<robinhood>::Go<Identity64Pack>(keys,loadfactor);
        break;
    default:
        printf("unrecognized hasher index %d \n", hasher);
        printusage(argv[0]);
        return -1;
    }
    return 0;
}
