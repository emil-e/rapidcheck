#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/newgen/Create.h"

#include "util/ArbitraryRandom.h"

using namespace rc;

TEST_CASE("newgen::just") {
    prop("returns value without shrinks regardless of input",
         [](int value, const Random &random, int size) {
             RC_ASSERT(newgen::just(value)(random, size) ==
                       shrinkable::just(value));
         });
}
