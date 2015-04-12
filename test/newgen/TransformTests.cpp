#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/newgen/Transform.h"
#include "rapidcheck/newgen/Create.h"

#include "util/GenUtils.h"
#include "util/Predictable.h"
#include "util/Generators.h"
#include "util/ShrinkableUtils.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("newgen::map") {
    newprop(
        "maps the shrinkable returned by the generator",
        [](const Shrinkable<int> &shrinkable) {
            const auto mapper = [](int x) { return x * x; };
            const auto mapped = newgen::map(
                Gen<int>(fn::constant(shrinkable)),
                mapper)(Random(), 0);
            RC_ASSERT(shrinkable::map(shrinkable, mapper) == mapped);
        });

    newprop(
        "forwards the parameter to the generator",
        [](const GenParams &params) {
            const auto gen = newgen::map(
                genPassedParams(),
                [](GenParams &&x) { return std::move(x); });
            RC_ASSERT(gen(params.random, params.size).value() == params);
        });

    SECTION("works with non-copyable types") {
        const auto value = newgen::map(
            newgen::arbitrary<NonCopyable>(),
            [](NonCopyable &&nc) { return std::move(nc); })(
                Random(), 0).value();
        REQUIRE(isArbitraryPredictable(value));
    }

    newprop(
        "uses newgen::arbitrary if no generator is specified",
        [](const GenParams &params) {
            const auto value = newgen::map<Predictable>(
                [](Predictable &&x) { return std::move(x); })(
                    params.random, params.size).value();
            RC_ASSERT(isArbitraryPredictable(value));
        });

    newprop(
        "finds minimum where string represtation of unsigned integer must be"
        " longer than some value",
        [](const Random &random) {
            const auto gen = newgen::map(
                newgen::arbitrary<unsigned int>(),
                [](unsigned int x) { return std::to_string(x); });
            const auto n = *newgen::inRange(2, 7);
            std::string expected(n, '0');
            expected[0] = '1';
            const auto result = searchGen(
                random, gen::kNominalSize, gen,
                [=](const std::string &x) { return x.size() >= n; });
            RC_ASSERT(result == expected);
        });
}

TEST_CASE("newgen::cast") {
    newprop(
        "casting to a larger type and then back yields original",
        [](const Shrinkable<uint8_t> &shrinkable) {
            const Gen<uint8_t> gen(fn::constant(shrinkable));
            const auto cast = newgen::cast<uint8_t>(newgen::cast<int>(gen));
            RC_ASSERT(cast(Random(), 0) == shrinkable);
        });
}

TEST_CASE("newgen::suchThat") {
    newprop(
        "generated value always matches predicate",
        [](const GenParams &params) {
            const auto gen = newgen::suchThat(
                newgen::arbitrary<int>(),
                [](int x) { return (x % 2) == 0; });
            const auto shrinkable = gen(params.random, params.size);
            newOnAnyPath(
                shrinkable,
                [](const Shrinkable<int> &value,
                   const Shrinkable<int> &shrink) {
                    RC_ASSERT((value.value() % 2) == 0);
                });
        });

    newprop(
        "if predicate returns true for every value, returned shrinkable is"
        " unchanged",
        [](const GenParams &params, const Shrinkable<int> &shrinkable) {
            const Gen<int> underlying(fn::constant(shrinkable));
            const auto gen = newgen::suchThat(underlying, fn::constant(true));
            RC_ASSERT(underlying(params.random, params.size) ==
                      gen(params.random, params.size));
        });

    newprop(
        "throws GenerationFailure if value cannot be generated",
        [](const GenParams &params) {
            const auto gen = newgen::suchThat(newgen::just<int>(0),
                                              fn::constant(false));
            const auto shrinkable = gen(params.random, params.size);
            try {
                shrinkable.value();
            } catch (const GenerationFailure &e) {
                RC_SUCCEED("Threw GenerationFailure");
            }
            RC_FAIL("Didn't throw GenerationFailure");
        });

    newprop(
        "passes the passed size to the underlying generator on the first try",
        [] {
            const auto size = *newgen::inRange<int>(0, 2000);
            const auto gen = newgen::suchThat(genPassedParams(),
                                              fn::constant(true));
            const auto params = gen(Random(), size).value();
            RC_ASSERT(params.size == size);
        });

    newprop(
        "uses newgen::arbitrary if no generator is specified",
        [](const GenParams &params) {
            const auto value = newgen::suchThat<Predictable>(
                fn::constant(true))(params.random, params.size).value();
            RC_ASSERT(isArbitraryPredictable(value));
        });

    SECTION("works with non-copyable types") {
        const auto value = newgen::suchThat(
            newgen::arbitrary<NonCopyable>(),
            fn::constant(true))(Random(), 0).value();
        REQUIRE(isArbitraryPredictable(value));
    }
}

TEST_CASE("newgen::resize") {
    newprop(
        "always uses the specified size",
        [](const GenParams &params) {
            const auto size = *newgen::inRange<int>(0, 2000);
            const auto gen = newgen::resize(size, genPassedParams());
            const auto value = gen(params.random, params.size).value();
            RC_ASSERT(value.size == size);
        });

    newprop(
        "passes through random generator unchanged",
        [](const GenParams &params) {
            const auto gen = newgen::resize(0, genPassedParams());
            const auto value = gen(params.random, params.size).value();
            RC_ASSERT(value.random == params.random);
        });
}

TEST_CASE("newgen::scale") {
    newprop(
        "scales the size by the specified factor",
        [](const GenParams &params) {
            const auto gen = newgen::scale(2.0, genPassedParams());
            const auto value = gen(params.random, params.size).value();
            RC_ASSERT(value.size == params.size * 2);
        });

    newprop(
        "passes through random generator unchanged",
        [](const GenParams &params) {
            const auto gen = newgen::scale(2.0, genPassedParams());
            const auto value = gen(params.random, params.size).value();
            RC_ASSERT(value.random == params.random);
        });
}

TEST_CASE("newgen::noShrink") {
    newprop(
        "returned shrinkable has expected value",
        [](const GenParams &params, int x) {
            const auto gen = newgen::noShrink(newgen::just(x));
            const auto value = gen(params.random, params.size).value();
            RC_ASSERT(value == x);
        });

    newprop(
        "returned shrinkable has no shrinks",
        [](const GenParams &params) {
            const auto gen = newgen::noShrink(newgen::arbitrary<int>());
            const auto shrinkable = gen(params.random, params.size);
            RC_ASSERT(!shrinkable.shrinks().next());
        });

    newprop(
        "passes generation params unchanged",
        [](const GenParams &params) {
            const auto gen = newgen::noShrink(genPassedParams());
            const auto value = gen(params.random, params.size).value();
            RC_ASSERT(value == params);
        });
}

TEST_CASE("newgen::withSize") {
    newprop(
        "passes the current size to the callable",
        [](const GenParams &params) {
            const auto gen = newgen::withSize([](int size) {
                return newgen::just(size);
            });
            const auto value = gen(params.random, params.size).value();
            RC_ASSERT(value == params.size);
        });

    newprop(
        "generates what the returned generator generates",
        [](const GenParams &params, int x) {
            const auto gen = newgen::withSize([=](int size) {
                return newgen::just(x);
            });
            const auto shrinkable = gen(params.random, params.size);
            RC_ASSERT(shrinkable == shrinkable::just(x));
        });

    newprop(
        "passes generation params unchanged",
        [](const GenParams &params) {
            const auto gen = newgen::withSize([](int size) {
                return genPassedParams();
            });
            const auto value = gen(params.random, params.size).value();
            RC_ASSERT(value == params);
        });
}
