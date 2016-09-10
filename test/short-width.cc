// Exhaustively testing properties of hash families by instantiating
// them on fixed with integers of small widths.
#include <cmath>
#include <cstdint>

#include <algorithm>
#include <array>
#include <iostream>
#include <vector>

using namespace std;

template <size_t WIDTH>
struct Nat {
  const static size_t CARDINALITY = static_cast<size_t>(1) << WIDTH;
  typedef uint64_t Rep;
  Rep value_;
  Nat(Rep value = 0) : value_(value & (CARDINALITY - 1)) {}
  operator Rep() const { return value_; }
  template <typename F>
  static void ForEach(const F& f) {
    for (Rep x = 0; x < CARDINALITY; ++x) {
      f(x);
    }
  }
};

template <typename HashFamily>
double Universality() {
  using Domain = typename HashFamily::Domain;
  using Range = typename HashFamily::Range;
  using Rep = typename HashFamily::Rep;
  size_t result = 0;
  Domain::ForEach([&](Domain x) {
    cout << x << " " << flush;
    Domain::ForEach([&](Domain y) {
      if (y > x) {
        size_t count = 0;
        Rep::ForEach([&](Rep h) {
          const HashFamily hasher(h);
          count += (hasher(x) == hasher(y));
        });
        result = max(result, count);
      }
    });
  });
  return static_cast<double>(result * Range::CARDINALITY) /
         static_cast<double>(Rep::CARDINALITY);
}

// A hash family H is epsilon-almost-delta-universal if for all x, y,
// and z, if x != y, the probability that a randomly chosen h from H
// will have such that h(x) - h(y) = z is epsilon/Z or less, where Z
// is the number of elements in the co-domain.
template <typename HashFamily>
double DeltaUniversality() {
  using Domain = typename HashFamily::Domain;
  using Range = typename HashFamily::Range;
  using Rep = typename HashFamily::Rep;
  size_t result = 0;
  array<size_t, Range::CARDINALITY> count;
  Domain::ForEach([&](Domain x) {
    cout << x << " " << flush;
    Domain::ForEach([&](Domain y) {
      if (y > x) {
        fill(count.begin(), count.end(), 0);
        Rep::ForEach([&](Rep h) {
          const HashFamily hasher(h);
          const Range d = hasher(x) - hasher(y);
          ++count[d];
        });
        result = max(result, *max_element(count.begin(), count.end()));
      }
    });
  });
  return static_cast<double>(result * Range::CARDINALITY) /
         static_cast<double>(Rep::CARDINALITY);
}

template <typename HashFamily>
double StrongUniversality() {
  using Domain = typename HashFamily::Domain;
  using Range = typename HashFamily::Range;
  using Rep = typename HashFamily::Rep;
  size_t result = 0;
  vector<size_t> count(Range::CARDINALITY * Range::CARDINALITY);
  Domain::ForEach([&](Domain x) {
    cout << x << " " << flush;
    Domain::ForEach([&](Domain y) {
      if (y > x) {
        fill(count.begin(), count.end(), 0);
        Rep::ForEach([&](Rep h) {
          const HashFamily hasher(h);
          ++count[hasher(x) * Range::CARDINALITY + hasher(y)];
        });
        result = max(result, *max_element(count.begin(), count.end()));
      }
    });
  });

  return static_cast<double>(result * Range::CARDINALITY * Range::CARDINALITY) /
         static_cast<double>(Rep::CARDINALITY);
}

// Return all bijections from a HashFamily
template <typename HashFamily>
vector<HashFamily> Bijections() {
  using Domain = typename HashFamily::Domain;
  using Range = typename HashFamily::Range;
  using Rep = typename HashFamily::Rep;
  vector<HashFamily> result;
  Rep::ForEach([&](Rep h) {
    const HashFamily hasher(h);
    vector<bool> present(Range::CARDINALITY, false);
    Domain::ForEach([&](Domain x) { present[hasher(x)] = true; });
    if (all_of(present.begin(), present.end(), [](bool x) { return x; })) {
      result.push_back(hasher);
    }
  });
  return result;
}

// The classic 2-almost universal family from Dietzfelbinger et al.
template <size_t WIDTH>
struct MultiplyOdd {
  typedef Nat<WIDTH> Domain;
  typedef Nat<WIDTH> Range;
  typedef Nat<WIDTH - 1> Rep;
  Domain h_;
  explicit MultiplyOdd(Rep h) : h_(1 + 2 * h) {}
  Range operator()(Domain x) const { return static_cast<Domain>(h_) * x; }
};

// The classic strongly-universal family from Dietzfelbinger et al.
template <size_t WIDTH>
struct MultiplyAdd {
  typedef Nat<WIDTH> Domain;
  typedef Nat<WIDTH> Range;
  typedef Nat<4 * WIDTH> Rep;
  Nat<2 * WIDTH> m_, a_;
  explicit MultiplyAdd(Rep h) : m_(h), a_(h >> (2 * WIDTH)) {}
  Range operator()(Domain x) const {
    return ((static_cast<Nat<2 * WIDTH>>(x) * m_) + a_) >> WIDTH;
  }
};

// Like MultiplyOdd, but Woelfel noticed you don't need the high bit
template <size_t WIDTH>
struct MultiplyOddShort {
  typedef Nat<WIDTH> Domain;
  typedef Nat<WIDTH> Range;
  typedef Nat<WIDTH - 2> Rep;
  Domain h_;
  explicit MultiplyOddShort(Rep h) : h_(1 + 2 * h) {}
  Range operator()(Domain x) const { return static_cast<Domain>(h_) * x; }
};

// Like MultiplyAdd, but with only the multiplication makes it 2-almost delta
// universal
template <size_t WIDTH>
struct MultiplyBig {
  typedef Nat<WIDTH> Domain;
  typedef Nat<WIDTH> Range;
  typedef Nat<2 * WIDTH> Rep;
  Rep h_;
  explicit MultiplyBig(Rep h) : h_(h) {}
  Range operator()(Domain x) const {
    return (static_cast<Rep>(x) * h_) >> WIDTH;
  }
};

// Technically, Dietzfelbinger showed you don't need the high bit from the
// multiplier. This was explicitly stated by Woelfel.
template <size_t WIDTH>
struct MultiplyBigShort {
  typedef Nat<WIDTH> Domain;
  typedef Nat<WIDTH> Range;
  typedef Nat<2 * WIDTH - 1> Rep;
  Rep h_;
  explicit MultiplyBigShort(Rep h) : h_(h) {}
  Range operator()(Domain x) const {
    return (static_cast<Rep>(x) * h_) >> (WIDTH - 1);
  }
};

// Pseudo dot product on one input. This is sqrt{U} almost delta universal
template <size_t WIDTH>
struct PDP1 {
  typedef Nat<2 * WIDTH> Domain;
  typedef Nat<2 * WIDTH> Range;
  typedef Nat<2 * WIDTH> Rep;
  Nat<WIDTH> a_, b_;
  explicit PDP1(Rep h) : a_(h), b_(h >> WIDTH) {}
  Range operator()(Domain x) const {
    const Nat<WIDTH> p(x), q(x >> WIDTH);
    const Nat<WIDTH> s = p + a_, t = q + b_;
    const Range f(s), g(t);
    return f * g;
  }
};

// Pseudo dot product with extra additive term to make it sqrt{U} almost
// strongly universal
template <size_t WIDTH>
struct StrongPDP {
  typedef Nat<2 * WIDTH> Domain;
  typedef Nat<2 * WIDTH> Range;
  typedef Nat<4 * WIDTH> Rep;
  Nat<WIDTH> a_, b_;
  Nat<2 * WIDTH> c_;
  explicit StrongPDP(Rep h) : a_(h), b_(h >> WIDTH), c_(h >> (2 * WIDTH)) {}
  Range operator()(Domain x) const {
    const Nat<WIDTH> p(x), q(x >> WIDTH);
    const Nat<WIDTH> s = p + a_, t = q + b_;
    const Range f(s), g(t);
    const Range fg = f * g;
    return fg + c_;
  }
};

// Just the low bits of a PDP. This is not a good hash function
template <size_t WIDTH>
struct PDP1bottom {
  typedef Nat<2 * WIDTH> Domain;
  typedef Nat<WIDTH> Range;
  typedef Nat<2 * WIDTH> Rep;
  Nat<WIDTH> a_, b_;
  explicit PDP1bottom(Rep h) : a_(h), b_(h >> WIDTH) {}
  Range operator()(Domain x) const {
    const Range p(x), q(x >> WIDTH);
    const Range s = p + a_, t = q + b_;
    const Range ans(s * t);
    return ans;
  }
};

// Just the high bits of a PDP. This is not a good hash function
template <size_t WIDTH>
struct PDP1top {
  typedef Nat<2 * WIDTH> Domain;
  typedef Nat<WIDTH> Range;
  typedef Nat<2 * WIDTH> Rep;
  Nat<WIDTH> a_, b_;
  explicit PDP1top(Rep h) : a_(h), b_(h >> WIDTH) {}
  Range operator()(Domain x) const {
    const Range p(x), q(x >> WIDTH);
    const Range s = p + a_, t = q + b_;
    const Domain f(s), g(t);
    return (f * g) >> WIDTH;
  }
};

// Just the middle bits of a PDP. This is not a good hash function
template <size_t WIDTH>
struct PDP1mid {
  typedef Nat<2 * WIDTH> Domain;
  typedef Nat<WIDTH> Range;
  typedef Nat<2 * WIDTH> Rep;
  Nat<WIDTH> a_, b_;
  explicit PDP1mid(Rep h) : a_(h), b_(h >> WIDTH) {}
  Range operator()(Domain x) const {
    const Nat<WIDTH> p(x), q(x >> WIDTH);
    const Nat<WIDTH> s = p + a_, t = q + b_;
    const Domain f(s), g(t);
    return (f * g) >> (WIDTH / 2);
  }
};

// PDP with the domain having twice as many bits - only the first ones are
// PDPed, the rest ar added in. sqrt{U} universal, like any other addition with
// a delta-universal family.
template <size_t WIDTH>
struct PDP2 {
  typedef Nat<4 * WIDTH> Domain;
  typedef Nat<2 * WIDTH> Range;
  typedef Nat<2 * WIDTH> Rep;
  Nat<WIDTH> a_, b_;
  explicit PDP2(Rep h) : a_(h), b_(h >> WIDTH) {}
  Range operator()(Domain x) const {
    const Nat<WIDTH> p(x), q(x >> WIDTH);
    const Nat<WIDTH> s = p + a_, t = q + b_;
    const Range f(s), g(t);
    const Range fg = f * g;

    const Range i(x >> (2 * WIDTH));

    return fg + i;
  }
};

// Another demonstration of addition turning almost-delta universality into
// universality, this time with the delta universal family from Dietzfelbinger
template <size_t WIDTH>
struct MultiplyBigLeft {
  typedef Nat<2 * WIDTH> Domain;
  typedef Nat<WIDTH> Range;
  typedef Nat<2 * WIDTH> Rep;
  Rep h_;
  explicit MultiplyBigLeft(Rep h) : h_(h) {}
  Range operator()(Domain x) const {
    Rep xp = x >> WIDTH;
    Rep xph = xp * h_;
    Range xphs = xph >> WIDTH;
    Range xs = x & (Range::CARDINALITY - 1);
    Range ans = xphs + xs;
    return ans;
  }
};

// Like MultiplyOdd, but uses the high bits (and the most significant low bit)
// from the product. Might be a good hash function, but likely slower than
// MultiplyOdd.
template <size_t WIDTH>
struct MultiplyOddTop {
  typedef Nat<WIDTH> Domain;
  typedef Nat<WIDTH> Range;
  typedef Nat<WIDTH - 1> Rep;
  Domain h_;
  explicit MultiplyOddTop(Rep h) : h_(Rep::CARDINALITY | h) {}
  Range operator()(Domain x) const {
    return (static_cast<Nat<2 * WIDTH>>(h_) * static_cast<Nat<2 * WIDTH>>(x)) >>
           (WIDTH - 1);
  }
};

// The full power of MultiplyOddShort includes the reduction to the range size
// via right shift. 2-almost universal.
template <size_t DOMAIN_WIDTH, size_t RANGE_WIDTH>
struct MultiplyOddShift {
  static_assert(DOMAIN_WIDTH >= RANGE_WIDTH, "no negative shifts");
  typedef Nat<DOMAIN_WIDTH> Domain;
  typedef Nat<RANGE_WIDTH> Range;
  typedef Nat<DOMAIN_WIDTH - 2> Rep;
  Domain h_;
  explicit MultiplyOddShift(Rep h) : h_(1 + 2 * h) {}
  Range operator()(Domain x) const {
    return (static_cast<Domain>(h_) * x) >> (DOMAIN_WIDTH - RANGE_WIDTH);
  }
};

// Checking to see if MultiplyOddTop is flexible like MultiplyOdd, in that the
// low bits (not the high ones) are universal. Still under investigation.
template <size_t DOMAIN_WIDTH, size_t RANGE_WIDTH>
struct MultiplyOddTopMask {
  static_assert(DOMAIN_WIDTH >= RANGE_WIDTH, "no negative shifts");
  typedef Nat<DOMAIN_WIDTH> Domain;
  typedef Nat<RANGE_WIDTH> Range;
  typedef Nat<DOMAIN_WIDTH - 1> Rep;
  Domain h_;
  explicit MultiplyOddTopMask(Rep h) : h_(Rep::CARDINALITY | h) {}
  Range operator()(Domain x) const {
    return ((static_cast<Nat<2 * DOMAIN_WIDTH>>(h_) *
             static_cast<Nat<2 * DOMAIN_WIDTH>>(x)) >>
            (DOMAIN_WIDTH - 1)) &
           ((1ull << RANGE_WIDTH) - 1);
  }
};

// Printing the bits of a uint64_t, lowest first.
string Bits(uint64_t x) {
  string result;
  while (x > 0) {
    result += (x & 1) ? '1' : '.';
    x = x / 2;
  }
  return result;
}

int main() {
  typedef MultiplyBigShort<8> HashFamily;
  const auto du = DeltaUniversality<HashFamily>();
  cout << endl
       << du << " / " << HashFamily::Range::CARDINALITY << endl;
  const auto ans = Bijections<HashFamily>();
  for (const auto p : ans) {
    cout << p.h_.value_ << '\t' << Bits(p.h_.value_) << endl;
  }
  cout << endl
       << ans.size() << " / " << HashFamily::Rep::CARDINALITY << endl;
}
