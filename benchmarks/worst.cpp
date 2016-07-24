#include <cstdint>
#include <cmath>

#include <string>
#include <limits>
#include <algorithm>
#include <vector>
#include <numeric>
#include <utility>
#include <iostream>
#include <iomanip>

using namespace std;

#include "hashpack.h"
#include "buckets.hpp"

template<typename T>
string bits(T x) {
  string ans;
  for (int i = 0; i < numeric_limits<T>::digits; ++i) {
    ans += (x % 2) ? '1' : '0';
    x >>= 1;
  }
  reverse(ans.begin(), ans.end());
  return ans;
}

template<typename T>
double inverseOfFraction(T x) {
  double ans = 0;
  for (int i = 1; i <= numeric_limits<T>::digits; ++i) {
    ans += (x % 2) * pow(2,-1+i-numeric_limits<T>::digits);
    x >>= 1;
  }
  return 1/ans;
}

int main() {
  vector<uint8_t> data(256);
  iota(data.begin(), data.end(), 0);
  vector<pair<size_t, uint16_t> > functions;
  for (uint32_t i = 0; i < (1 << 16); ++i) {
      functions.emplace_back(
          SearchTime(MultiplyOnly8Pack(i), Modulo64, 1 << 8, data), i);
  }
  sort(functions.begin(), functions.end(),
       std::greater<pair<size_t, uint16_t>>());
  for (int i = 0; i < (1 << 16); ++i) {
      cout << dec << setw(10) << functions[i].first << setw(20)
           << bits(functions[i].second) << setw(10) << functions[i].second
           << setw(10) << inverseOfFraction(functions[i].second) << endl;
  }
}
