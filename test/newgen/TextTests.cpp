#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/newgen/Text.h"

#include "util/GenUtils.h"
#include "util/ShrinkableUtils.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("newgen::character") {
    newprop(
        "never generates null characters",
        [](const GenParams &params) {
            const auto gen = newgen::character<char>();
            onAnyPath(
                gen(params.random, params.size),
                [](const Shrinkable<char> &value,
                   const Shrinkable<char> &shrink) {
                    RC_ASSERT(shrink.value() != '\0');
                });
        });

    newprop(
        "first shrink is always 'a')",
        [](const GenParams &params) {
            const auto gen = newgen::character<char>();
            const auto shrinkable = gen(params.random, params.size);
            RC_PRE(shrinkable.value() != 'a');
            RC_ASSERT(shrinkable.shrinks().next()->value() == 'a');
        });
}

TEST_CASE("newgen::arbitrary<std::string>") {
    newprop(
        "finds minimum where string must be longer than a certain length",
        [](const Random &random) {
            const auto n = *newgen::inRange<std::size_t>(0, 10);
            const auto size = *newgen::inRange<int>(50, 100);
            const auto result = searchGen(
                random, size, newgen::arbitrary<std::string>(),
                [=](const std::string &x) { return x.size() >= n; });
            std::string expected(n, 'a');
            RC_ASSERT(result == expected);
        });
}
