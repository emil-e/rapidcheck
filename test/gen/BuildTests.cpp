#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/GenUtils.h"
#include "util/Predictable.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("gen::construct") {
  prop("has tuple shrinking semantics",
       [](const GenParams &params) {
         const auto g1 = genFixedCountdown(*gen::inRange(0, 10));
         const auto g2 = genFixedCountdown(*gen::inRange(0, 10));
         const auto g3 = genFixedCountdown(*gen::inRange(0, 10));

         const auto tupleGen = gen::tuple(g1, g2, g3);
         const auto tupleShrinkable = tupleGen(params.random, params.size);

         const auto gen = gen::construct<std::tuple<int, int, int>>(g1, g2, g3);
         const auto shrinkable = gen(params.random, params.size);

         RC_ASSERT(shrinkable::immediateShrinks(shrinkable) ==
                   shrinkable::immediateShrinks(tupleShrinkable));
       });

  prop("passes correct size",
       [](const GenParams &params) {
         const auto gen = gen::construct<std::tuple<int, int, int>>(
             genSize(), genSize(), genSize());
         const auto value = gen(params.random, params.size).value();

         RC_ASSERT(value ==
                   std::make_tuple(params.size, params.size, params.size));
       });

  prop("passed random generators are unique",
       [](const GenParams &params) {
         const auto gen = gen::construct<std::tuple<Random, Random, Random>>(
           genRandom(), genRandom(), genRandom());
         const auto value = gen(params.random, params.size).value();

         RC_ASSERT(std::get<0>(value) != std::get<1>(value));
         RC_ASSERT(std::get<0>(value) != std::get<2>(value));
         RC_ASSERT(std::get<1>(value) != std::get<2>(value));
       });

  SECTION("works with non-copyable types") {
    const auto gen = gen::construct<std::tuple<NonCopyable, NonCopyable>>(
      gen::arbitrary<NonCopyable>(), gen::arbitrary<NonCopyable>());
    const auto value = gen(Random(), 0).value();

    RC_ASSERT(isArbitraryPredictable(std::get<0>(value)));
    RC_ASSERT(isArbitraryPredictable(std::get<1>(value)));
  }

  SECTION("chooses correct arbitrary instance when not given arguments") {
    const auto gen = gen::construct<std::tuple<Predictable, Predictable>,
                                    Predictable,
                                    Predictable>();
    const auto value = gen(Random(), 0).value();

    RC_ASSERT(isArbitraryPredictable(std::get<0>(value)));
    RC_ASSERT(isArbitraryPredictable(std::get<1>(value)));
  }
}
