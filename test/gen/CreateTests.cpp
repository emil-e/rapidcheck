#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/gen/Create.h"

#include "util/ArbitraryRandom.h"
#include "util/GenUtils.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("gen::just") {
  prop("returns value without shrinks regardless of params",
       [](const GenParams &params, int value) {
         RC_ASSERT(gen::just(value)(params.random, params.size) ==
                   shrinkable::just(value));
       });
}

TEST_CASE("gen::lazy") {
  SECTION("does not call callable on creation") {
    bool called = false;
    const auto gen = gen::lazy([&] {
      called = true;
      return gen::just(1);
    });
    REQUIRE_FALSE(called);
  }

  prop("passes parameters unchanged",
       [](const GenParams &params) {
         const auto gen = gen::lazy(&genPassedParams);
         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(value == params);
       });
}
