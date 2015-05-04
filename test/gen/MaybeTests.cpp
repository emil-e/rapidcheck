#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/GenUtils.h"
#include "util/ShrinkableUtils.h"
#include "util/Predictable.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("gen::maybe") {
  prop("if size is 0, generates only Nothing",
       [](const Random &random) {
         const auto shrinkable = gen::maybe(gen::arbitrary<int>())(random, 0);
         RC_ASSERT(shrinkable == shrinkable::just(Maybe<int>()));
       });

  prop("first shrink is always Nothing",
       [](const GenParams &params) {
         const auto gen = gen::maybe(gen::arbitrary<int>());
         onAnyPath(gen(params.random, params.size),
                   [](const Shrinkable<Maybe<int>> &value,
                      const Shrinkable<Maybe<int>> &shrink) {
                     if (value.value()) {
                       RC_ASSERT_FALSE(value.shrinks().next()->value());
                     }
                   });
       });

  prop("except for adding Nothing, shrinks are equal to value generator",
       [](const GenParams &params) {
         const auto valueGen = genFixedCountdown(*gen::inRange(0, 10));
         const auto valueShrinkable = valueGen(params.random, params.size);

         const auto gen = gen::maybe(valueGen);

         const auto maybeShrinkable =
             shrinkable::filter(gen(params.random, params.size),
                                [](const Maybe<int> &x) { return !!x; });
         RC_PRE(maybeShrinkable);
         const auto shrinkable = shrinkable::map(
             *maybeShrinkable, [](Maybe<int> &&x) { return std::move(*x); });

         RC_ASSERT(shrinkable == valueShrinkable);
       });

  prop("passes the correct size",
       [](const GenParams &params) {
         const auto gen = gen::maybe(genSize());
         const auto value = gen(params.random, params.size).value();
         RC_PRE(value);
         RC_ASSERT(*value == params.size);
       });

  prop("chooses correct Arbitrary instance",
       [](const GenParams &params) {
         const auto gen = gen::arbitrary<Maybe<NonCopyable>>();
         const auto value = gen(params.random, params.size).value();
         RC_PRE(value);
         RC_ASSERT(isArbitraryPredictable(*value));
       });
}
