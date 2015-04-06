#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/newgen/Exec.h"

#include "util/Predictable.h"
#include "util/GenUtils.h"

using namespace rc;
using namespace rc::newgen::detail;
using namespace rc::test;

TEST_CASE("newgen::exec") {
    newprop(
        "yields the same result as execRaw but without recipe",
        [](const GenParams &params) {
            using Tuple = std::tuple<int, int, int>;

            const auto callable = [](const FixedCountdown<2> &a) {
                return std::make_tuple(
                    a.value,
                    *genFixedCountdown(2),
                    *genFixedCountdown(2));
            };

            const auto expected = newgen::map(
                execRaw(callable), [](const std::pair<Tuple, Recipe> &p) {
                    return p.first;
                })(params.random, params.size);

            const auto actual = newgen::exec(callable)(params.random,
                                                       params.size);

            RC_ASSERT(actual == expected);
        });

    SECTION("works with non-copyable types") {
        auto shrinkable = newgen::exec([=](NonCopyable nc) {
            return std::move(nc);
        })(Random(), 0);
        REQUIRE(isArbitraryPredictable(shrinkable.value()));
    }
}
