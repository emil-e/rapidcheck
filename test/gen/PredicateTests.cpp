#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/GenUtils.h"
#include "util/ShrinkableUtils.h"
#include "util/Predictable.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("gen::nonEmpty") {
  prop("never generates empty values",
       [](const GenParams &params) {
         const auto gen = gen::nonEmpty(gen::string<std::string>());
         onAnyPath(gen(params.random, params.size),
                   [](const Shrinkable<std::string> &value,
                      const Shrinkable<std::string> &shrink) {
                     RC_ASSERT(!value.value().empty());
                   });
       });

  prop("uses correct arbitrary instance",
       [](const GenParams &params) {
         const auto gen = gen::nonEmpty<std::vector<Predictable>>();
         const auto value = gen(params.random, params.size).value();
         REQUIRE(std::all_of(
             begin(value),
             end(value),
             [](const Predictable &p) { return isArbitraryPredictable(p); }));
       });
}
