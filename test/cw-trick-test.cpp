#include <iomanip>
#include <iostream>

using namespace std;

extern "C" {
#include "cw-trick.h"
#include "util.h"
}

ostream& cerr128(uint128_t x) {
    return cerr << hex << "0x" << static_cast<uint64_t>(x >> 64) << " 0x"
                << static_cast<uint64_t>((x << 64) >> 64);
}

int main() {
    for (int i = 0; i < 50000000; ++i) {
        uint128_t x = get128rand() % ((CW_PRIME - 1) * (CW_PRIME - 1));
        if ((x % CW_PRIME) != cwmod(x)) {
            cerr128(x) << endl;
            cerr128(x % CW_PRIME) << endl;
            cerr << "0x" << hex << cwmod(x) << endl;
            return 1;
        }
    }
}
