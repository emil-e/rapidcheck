#pragma once

#include "rapidcheck/Random.h"
#include "rapidcheck/gen/Arbitrary.h"
#include "rapidcheck/gen/Tuple.h"
#include "rapidcheck/gen/Container.h"

namespace rc {

// This will only generate Randoms with no next() called.
template <>
struct Arbitrary<Random> {
  static Gen<Random> arbitrary() {
    return gen::map(gen::pair(gen::arbitrary<Random::Key>(),
                              gen::arbitrary<std::vector<bool>>()),
                    [](const std::pair<Random::Key, std::vector<bool>> &p) {
                      Random random(p.first);
                      for (bool x : p.second) {
                        if (!x)
                          random.split();
                        else
                          random = random.split();
                      }
                      return random;
                    });
  }
};

extern template struct Arbitrary<Random>;

namespace test {

// This will also generate Randoms where next() has been called
Gen<Random> trulyArbitraryRandom();

} // namespace test
} // namespace rc
