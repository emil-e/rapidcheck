#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/fn/Common.h"

#include "util/Logger.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("fn::constant") {
  SECTION("does not copy element when constructing if rvalue") {
    Logger logger("foobar");
    const auto f = fn::constant(std::move(logger));
    const auto value = f();
    REQUIRE(value.numberOf("copy constructed") == 1);
  }

  prop("returns same value regardless of arguments",
       [](const std::string &value, int a, int b, int c) {
         const auto f = fn::constant(value);
         RC_ASSERT(f() == value);
         RC_ASSERT(f(a) == value);
         RC_ASSERT(f(a, b) == value);
         RC_ASSERT(f(a, b, c) == value);
       });
}
