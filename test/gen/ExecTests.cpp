#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/gen/Exec.h"

#include "util/Predictable.h"
#include "util/GenUtils.h"

using namespace rc;
using namespace rc::gen::detail;
using namespace rc::test;

TEST_CASE("gen::exec") {
  prop("yields the same result as execRaw but without recipe",
       [](const GenParams &params) {
         using Tuple = std::tuple<int, int, int>;

         const auto callable = [](const FixedCountdown<2> &a) {
           return std::make_tuple(
               a.value, *genFixedCountdown(2), *genFixedCountdown(2));
         };

         const auto expected = gen::map(execRaw(callable),
                                        [](const std::pair<Tuple, Recipe> &p) {
                                          return p.first;
                                        })(params.random, params.size);

         const auto actual = gen::exec(callable)(params.random, params.size);

         RC_ASSERT(actual == expected);
       });

  SECTION("works with non-copyable types") {
    auto shrinkable =
        gen::exec([=](NonCopyable nc) { return std::move(nc); })(Random(), 0);
    REQUIRE(isArbitraryPredictable(shrinkable.value()));
  }

  prop("equivalent to monadic bind",
       [] {
         const auto a = *gen::inRange(0, 5);
         const auto b = *gen::inRange(0, 5);

         const auto execGen = gen::exec([=] {
           int x1 = *genFixedCountdown(a);
           int x2 = *genFixedCountdown(b);
           return std::make_pair(x1, x2);
         });

         const auto bindGen = gen::mapcat(
             genFixedCountdown(a),
             [=](int x1) {
               return gen::map(genFixedCountdown(b),
                               [=](int x2) { return std::make_pair(x1, x2); });
             });

         RC_ASSERT(execGen(Random(), 0) == bindGen(Random(), 0));
       });
}
