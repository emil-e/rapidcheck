#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/newgen/Create.h"

#include "util/ArbitraryRandom.h"
#include "util/GenUtils.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("newgen::just") {
    newprop(
        "returns value without shrinks regardless of params",
        [](const GenParams &params, int value) {
            RC_ASSERT(newgen::just(value)(params.random, params.size) ==
                      shrinkable::just(value));
        });
}

TEST_CASE("newgen::lazy") {
    SECTION("does not call callable on creation") {
        bool called = false;
        const auto gen = newgen::lazy([&]{
            called = true;
            return newgen::just(1);
        });
        REQUIRE_FALSE(called);
    }

    newprop(
        "passes parameters unchanged",
        [](const GenParams &params) {
            const auto gen = newgen::lazy(&genPassedParams);
            const auto value = gen(params.random, params.size).value();
            RC_ASSERT(value == params);
        });
}
