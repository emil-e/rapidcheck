#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/shrinkable/Create.h"

#include "util/Logger.h"
#include "util/Generators.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("shrinkable::lambda") {
    SECTION("calls the value callable when value is called each time") {
        int calledTimes = 0;
        const auto shrinkable = shrinkable::lambda(
            [&] { return calledTimes++; },
            [] { return Seq<Shrinkable<int>>(); });

        REQUIRE(shrinkable.value() == 0);
        REQUIRE(shrinkable.value() == 1);
    }

    SECTION("calls shrinks() of the implementation object") {
        auto shrinks = seq::just(shrinkable::just(123, Seq<Shrinkable<int>>()));
        int calledTimes = 0;
        const auto shrinkable = shrinkable::lambda(
            [] { return 0; },
            [&] {
                calledTimes++;
                return shrinks;
            });

        REQUIRE(calledTimes == 0);
        REQUIRE(shrinkable.shrinks() == shrinks);
        REQUIRE(calledTimes == 1);
        REQUIRE(shrinkable.shrinks() == shrinks);
        REQUIRE(calledTimes == 2);
    }
}

TEST_CASE("shrinkable::just") {
    prop("creates a shrinkable which returns the given value and shrinks",
         [](int value, const Seq<Shrinkable<int>> &shrinks) {
             const auto shrinkable = shrinkable::just(value, shrinks);
             RC_ASSERT(shrinkable.value() == value);
             RC_ASSERT(shrinkable.shrinks() == shrinks);
         });

    SECTION("does not copy on construction if rvalues") {
        const auto shrinkable = shrinkable::just(
            Logger("value"),
            seq::just(shrinkable::just(Logger("shrink"))));
        const auto value = shrinkable.value();
        REQUIRE(value.numberOf("copy constructed") == 1);
        const auto shrink = shrinkable.shrinks().next();
        REQUIRE(shrink);
        REQUIRE(shrink->value().numberOf("copy constructed") == 2);
    }
}
