#include "ArbitraryRandom.h"

#include "rapidcheck/gen/Numeric.h"

// TODO clean up, formalize

namespace rc {

template struct Arbitrary<Random>;

namespace test {

Gen<Random> trulyArbitraryRandom() {
  return gen::map(
      gen::pair(gen::arbitrary<Random>(), gen::inRange<int>(0, 10000)),
      [](std::pair<Random, int> &&p) {
        auto nexts = p.second;
        while (nexts-- > 0) {
          p.first.next();
        }
        return p.first;
      });
}

} // namespace test
} // namespace rc
