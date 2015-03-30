#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/newgen/Transform.h"
#include "rapidcheck/newgen/Create.h"

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
            newgen::arbitrary<NonCopyable>())(Random(), 0).value();
        REQUIRE(isArbitraryPredictable(value));
    }

    prop("uses newgen::arbitrary if no generator is specified",
         [](const GenParams &params) {
             const auto value = newgen::map<Predictable>(
                 [](Predictable &&x) { return std::move(x); })(
                     params.random, params.size).value();
             RC_ASSERT(isArbitraryPredictable(value));
         });

    prop("finds minimum where string represtation of unsigned integer must be"
         " longer than some value",
         [](const Random &random) {
             const auto gen = newgen::map(
                 [](unsigned int x) { return std::to_string(x); },
                 newgen::arbitrary<unsigned int>());
             const auto n = *gen::ranged(2, 7);
             std::string expected(n, '0');
             expected[0] = '1';
             const auto result = searchGen(
                 random, gen::kNominalSize, gen,
                 [=](const std::string &x) { return x.size() >= n; });
             RC_ASSERT(result == expected);
         });
}

TEST_CASE("newgen::cast") {
    prop("casting to a larger type and then back yields original",
         [](const Shrinkable<uint8_t> &shrinkable) {
             const Gen<uint8_t> gen(fn::constant(shrinkable));
             const auto cast = newgen::cast<uint8_t>(newgen::cast<int>(gen));
             RC_ASSERT(cast(Random(), 0) == shrinkable);
         });
}
