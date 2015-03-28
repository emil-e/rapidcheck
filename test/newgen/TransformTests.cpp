#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/newgen/Transform.h"

#include "util/GenUtils.h"
#include "util/Predictable.h"
#include "util/Generators.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("newgen::map") {
    prop("maps the shrinkable returned by the generator",
         [](const Shrinkable<int> &shrinkable) {
             const auto mapper = [](int x) { return x * x; };
             const auto mapped = newgen::map(
                 mapper,
                 Gen<int>(fn::constant(shrinkable)))(Random(), 0);
             RC_ASSERT(shrinkable::map(mapper, shrinkable) == mapped);
         });

    prop("forwards the parameter to the generator",
         [](const GenParams &params) {
             const auto gen = newgen::map(
                 [](GenParams &&x) { return std::move(x); },
                 genPassedParams());
             RC_ASSERT(gen(params.random, params.size).value() == params);
         });

    SECTION("works with non-copyable types") {
        const auto value = newgen::map(
            [](NonCopyable &&nc) { return std::move(nc); },
            Gen<NonCopyable>([](const Random &, int) {
                return shrinkable::lambda([] {
                    NonCopyable nc;
                    nc.value = 1337;
                    nc.extra = 1337;
                    return nc;
                });
            }))(Random(), 0).value();
        RC_ASSERT(value.value == 1337);
        RC_ASSERT(value.extra == 1337);
    }
}

TEST_CASE("newgen::cast") {
    prop("casting to a larger type and then back yields original",
         [](const Shrinkable<uint8_t> &shrinkable) {
             const Gen<uint8_t> gen(fn::constant(shrinkable));
             const auto cast = newgen::cast<uint8_t>(newgen::cast<int>(gen));
             RC_ASSERT(cast(Random(), 0) == shrinkable);
         });
}
