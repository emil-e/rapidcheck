#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>
#include <rapidcheck/boost.h>

#include "util/ShrinkableUtils.h"
#include "util/GenUtils.h"
#include "util/Predictable.h"
#include "util/Box.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("gen::boost::optional") {
  prop("equivalent to gen::maybe",
       [](const GenParams &params) {
         const auto gen = gen::arbitrary<int>();
         const auto maybeShrinkable =
             gen::maybe(gen)(params.random, params.size);
         const auto optionalShrinkable = shrinkable::map(
             gen::boost::optional(gen)(params.random, params.size),
             [](const boost::optional<int> &x) -> Maybe<int> {
               if (x) {
                 return std::move(*x);
               } else {
                 return Nothing;
               }
             });

         assertEquivalent(maybeShrinkable, optionalShrinkable);
       });

  prop("chooses correct Arbitrary instance",
       [](const GenParams &params) {
         const auto gen = gen::arbitrary<boost::optional<NonCopyable>>();
         const auto value = gen(params.random, params.size).value();
         RC_PRE(value);
         RC_ASSERT(isArbitraryPredictable(*value));
       });
}

TEST_CASE("showValue(boost::optional)") {
  prop("shows present value using rc::show",
       [](const Box &x) {
         std::ostringstream actual;
         showValue(boost::make_optional(x), actual);
         std::ostringstream expected;
         showValue(x, expected);
         RC_ASSERT(actual.str() == expected.str());
       });

  SECTION("shows non-present value as 'boost::none'") {
    std::ostringstream os;
    showValue(boost::optional<int>(), os);
    REQUIRE(os.str() == "boost::none");
  }
}
