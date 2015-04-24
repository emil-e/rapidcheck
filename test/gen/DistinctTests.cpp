#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/gen/Distinct.h"
#include "rapidcheck/gen/Create.h"

#include "util/GenUtils.h"
#include "util/ShrinkableUtils.h"
#include "util/Predictable.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("gen::distinctFrom") {
  prop("value is never equal to the given one",
       [](const GenParams &params, int x) {
         const auto gen = gen::distinctFrom(x);
         onAnyPath(
             gen(params.random, params.size),
             [=](const Shrinkable<int> &value, const Shrinkable<int> &shrink) {
               RC_ASSERT(value.value() != x);
             });
       });

  prop("uses the correct generator when specified",
       [](const GenParams &params, int x) {
         const auto gen = gen::distinctFrom(gen::just(x), x - 1);
         RC_ASSERT(gen(params.random, params.size).value() == x);
       });

  prop("uses the correct arbitrary instance if generator not specified",
       [](const GenParams &params, const Predictable &x) {
         const auto gen = gen::distinctFrom(x);
         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(isArbitraryPredictable(value));
       });
}
