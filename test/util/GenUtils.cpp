#include "GenUtils.h"

#include "rapidcheck/Gen.h"
#include "rapidcheck/gen/Arbitrary.h"
#include "rapidcheck/shrinkable/Create.h"
#include "rapidcheck/shrinkable/Operations.h"

#include "util/ArbitraryRandom.h"
#include "util/ShrinkableUtils.h"

namespace rc {
namespace test {

Gen<int> genSize() {
  return [](const Random &random, int size) { return shrinkable::just(size); };
};

Gen<Random> genRandom() {
  return
      [](const Random &random, int size) { return shrinkable::just(random); };
}

Gen<int> genCountdown() {
  return [=](const Random &random, int size) {
    int n = Random(random).next() % (size + 1);
    return countdownShrinkable(n);
  };
}

Gen<int> genFixedCountdown(int value) {
  return [=](const Random &random, int size) {
    return countdownShrinkable(value);
  };
}

bool operator==(const GenParams &lhs, const GenParams &rhs) {
  return (lhs.random == rhs.random) && (lhs.size == rhs.size);
}

bool operator<(const GenParams &lhs, const GenParams &rhs) {
  return std::tie(lhs.random, lhs.size) < std::tie(rhs.random, rhs.size);
}

Gen<GenParams> genPassedParams() {
  return [](const Random &random, int size) {
    GenParams params;
    params.random = random;
    params.size = size;
    return shrinkable::just(params);
  };
}

std::ostream &operator<<(std::ostream &os, const GenParams &params) {
  os << "Random: " << params.random << std::endl;
  os << "Size: " << params.size << std::endl;
  return os;
}

} // namespace test

template struct Arbitrary<test::GenParams>;
template struct Arbitrary<test::PassedSize>;
template struct Arbitrary<test::PassedRandom>;

} // namespace rc
