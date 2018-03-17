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
void populate(ht & h, size_t N) {
    for(size_t i = 0; i < N; ++i) {
        h.emplace(i, i);
    }
}

int main() {
    const float loadfactor = 0.9;
    const uint64_t EMPTY = 0xFFFFFFFFFFFFFFFF;
    HashMap<uint64_t, uint32_t, Murmur64Pack, true> hm(16, EMPTY, loadfactor);
    populate(hm, 32);
    hm.debug_print();

}
