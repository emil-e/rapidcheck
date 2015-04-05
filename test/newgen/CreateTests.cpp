#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/newgen/Create.h"

#include "util/ArbitraryRandom.h"
#include "util/GenUtils.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("newgen::just") {
    prop("returns value without shrinks regardless of params",
         [](const GenParams &params, int value) {
             RC_ASSERT(newgen::just(value)(params.random, params.size) ==
                       shrinkable::just(value));
         });
}
