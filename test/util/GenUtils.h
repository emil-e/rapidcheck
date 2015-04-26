#pragma once

#include "rapidcheck/Gen.h"
#include "rapidcheck/Assertions.h"
#include "rapidcheck/gen/Numeric.h"
#include "rapidcheck/gen/Arbitrary.h"
#include "rapidcheck/shrinkable/Create.h"
#include "rapidcheck/shrinkable/Operations.h"

#include "util/ArbitraryRandom.h"

namespace rc {
namespace test {

// Generator which returns the passed size
Gen<int> genSize();

struct PassedSize {
  int value;
};

// Generator which returns the passed random.
Gen<Random> genRandom();

struct PassedRandom {
  Random value;
};

// Generates a number between 0 and the size (inclusive) that shrinks by
// counting down toward zero.
Gen<int> genCountdown();

// Generates a constant number which shrinks by count down towards zero
Gen<int> genFixedCountdown(int value);

template <int N>
struct FixedCountdown {
  FixedCountdown()
      : value(0) {}
  FixedCountdown(int x)
      : value(x) {}

  int value;
};

template <int N>
bool operator==(const FixedCountdown<N> &lhs, const FixedCountdown<N> &rhs) {
  return lhs.value == rhs.value;
}

template <int N>
std::ostream &operator<<(std::ostream &os, const FixedCountdown<N> &value) {
  os << value.value;
  return os;
}

struct GenParams {
  Random random;
  int size = 0;
};

bool operator==(const GenParams &lhs, const GenParams &rhs);
bool operator<(const GenParams &lhs, const GenParams &rhs);

// Generator which returns the passed generation params.
Gen<GenParams> genPassedParams();

std::ostream &operator<<(std::ostream &os, const GenParams &params);

// Tries to find a value which matches the predicate and then shrink that to the
// minimum value. Simplified version of what RapidCheck is all about.
template <typename T, typename Predicate>
T searchGen(const Random &random,
            int size,
            const Gen<T> &gen,
            Predicate predicate) {
  Random r(random);
  for (int tries = 0; tries < 100; tries++) {
    const auto shrinkable = gen(r.split(), size);
    const auto value = shrinkable.value();
    if (!predicate(value)) {
      continue;
    }
    return shrinkable::findLocalMin(shrinkable, predicate).first;
  }

  RC_DISCARD("Couldn't satisfy predicate");
}

} // namespace test

template <>
struct Arbitrary<test::GenParams> {
  static Gen<test::GenParams> arbitrary() {
    return gen::map(
        gen::pair(gen::arbitrary<Random>(), gen::inRange<int>(0, 200)),
        [](const std::pair<Random, int> &p) {
          test::GenParams params;
          params.random = p.first;
          params.size = p.second;
          return params;
        });
  }
};

template <>
struct Arbitrary<test::PassedSize> {
  static Gen<test::PassedSize> arbitrary() {
    return [](const Random &random, int size) {
      test::PassedSize sz;
      sz.value = size;
      return shrinkable::just(sz);
    };
  }
};

template <>
struct Arbitrary<test::PassedRandom> {
  static Gen<test::PassedRandom> arbitrary() {
    return [](const Random &random, int size) {
      test::PassedRandom rnd;
      rnd.value = random;
      return shrinkable::just(rnd);
    };
  }
};

template <int N>
struct Arbitrary<test::FixedCountdown<N>> {
  static Gen<test::FixedCountdown<N>> arbitrary() {
    return gen::map(test::genFixedCountdown(N),
                    [](int x) {
                      test::FixedCountdown<N> countdown;
                      countdown.value = x;
                      return countdown;
                    });
  }
};

extern template struct Arbitrary<test::GenParams>;
extern template struct Arbitrary<test::PassedSize>;
extern template struct Arbitrary<test::PassedRandom>;

} // namespace rc

namespace std {

template <>
struct hash<rc::test::GenParams> {
  using argument_type = rc::test::GenParams;
  using result_type = std::size_t;

  std::size_t operator()(const rc::test::GenParams &params) const {
    return std::hash<rc::Random>()(params.random) ^
        (std::hash<int>()(params.size) << 1);
  }
};

} // namespace std
