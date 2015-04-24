#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/gen/Transform.h"
#include "rapidcheck/gen/Create.h"

#include "util/GenUtils.h"
#include "util/Predictable.h"
#include "util/Generators.h"
#include "util/ShrinkableUtils.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("gen::map") {
  prop("maps the shrinkable returned by the generator",
       [](const Shrinkable<int> &shrinkable) {
         const auto mapper = [](int x) { return x * x; };
         const auto mapped =
             gen::map(Gen<int>(fn::constant(shrinkable)), mapper)(Random(), 0);
         RC_ASSERT(shrinkable::map(shrinkable, mapper) == mapped);
       });

  prop("forwards the parameter to the generator",
       [](const GenParams &params) {
         const auto gen = gen::map(genPassedParams(),
                                   [](GenParams &&x) { return std::move(x); });
         RC_ASSERT(gen(params.random, params.size).value() == params);
       });

  SECTION("works with non-copyable types") {
    const auto value =
        gen::map(gen::arbitrary<NonCopyable>(),
                 [](NonCopyable &&nc) { return std::move(nc); })(Random(), 0)
            .value();
    REQUIRE(isArbitraryPredictable(value));
  }

  prop("uses gen::arbitrary if no generator is specified",
       [](const GenParams &params) {
         const auto value = gen::map<Predictable>([](Predictable &&x) {
           return std::move(x);
         })(params.random, params.size)
                                .value();
         RC_ASSERT(isArbitraryPredictable(value));
       });

  prop(
      "finds minimum where string represtation of unsigned integer must be"
      " longer than some value",
      [](const Random &random) {
        const auto gen =
            gen::map(gen::arbitrary<unsigned int>(),
                     [](unsigned int x) { return std::to_string(x); });
        const auto n = *gen::inRange(2, 7);
        std::string expected(n, '0');
        expected[0] = '1';
        const auto result =
            searchGen(random,
                      kNominalSize,
                      gen,
                      [=](const std::string &x) { return x.size() >= n; });
        RC_ASSERT(result == expected);
      });
}

TEST_CASE("gen::cast") {
  prop("casting to a larger type and then back yields original",
       [](const Shrinkable<uint8_t> &shrinkable) {
         const Gen<uint8_t> gen(fn::constant(shrinkable));
         const auto cast = gen::cast<uint8_t>(gen::cast<int>(gen));
         RC_ASSERT(cast(Random(), 0) == shrinkable);
       });
}

TEST_CASE("gen::suchThat") {
  prop("generated value always matches predicate",
       [](const GenParams &params) {
         const auto gen = gen::suchThat(gen::arbitrary<int>(),
                                        [](int x) { return (x % 2) == 0; });
         const auto shrinkable = gen(params.random, params.size);
         onAnyPath(
             shrinkable,
             [](const Shrinkable<int> &value, const Shrinkable<int> &shrink) {
               RC_ASSERT((value.value() % 2) == 0);
             });
       });

  prop(
      "if predicate returns true for every value, returned shrinkable is"
      " unchanged",
      [](const GenParams &params, const Shrinkable<int> &shrinkable) {
        const Gen<int> underlying(fn::constant(shrinkable));
        const auto gen = gen::suchThat(underlying, fn::constant(true));
        RC_ASSERT(underlying(params.random, params.size) ==
                  gen(params.random, params.size));
      });

  prop("throws GenerationFailure if value cannot be generated",
       [](const GenParams &params) {
         const auto gen = gen::suchThat(gen::just<int>(0), fn::constant(false));
         const auto shrinkable = gen(params.random, params.size);
         try {
           shrinkable.value();
         } catch (const GenerationFailure &e) {
           RC_SUCCEED("Threw GenerationFailure");
         }
         RC_FAIL("Didn't throw GenerationFailure");
       });

  prop("passes the passed size to the underlying generator on the first try",
       [] {
         const auto size = *gen::inRange<int>(0, 2000);
         const auto gen = gen::suchThat(genPassedParams(), fn::constant(true));
         const auto params = gen(Random(), size).value();
         RC_ASSERT(params.size == size);
       });

  prop("uses gen::arbitrary if no generator is specified",
       [](const GenParams &params) {
         const auto value = gen::suchThat<Predictable>(fn::constant(true))(
                                params.random, params.size)
                                .value();
         RC_ASSERT(isArbitraryPredictable(value));
       });

  SECTION("works with non-copyable types") {
    const auto value = gen::suchThat(gen::arbitrary<NonCopyable>(),
                                     fn::constant(true))(Random(), 0)
                           .value();
    REQUIRE(isArbitraryPredictable(value));
  }
}

TEST_CASE("gen::resize") {
  prop("always uses the specified size",
       [](const GenParams &params) {
         const auto size = *gen::inRange<int>(0, 2000);
         const auto gen = gen::resize(size, genPassedParams());
         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(value.size == size);
       });

  prop("passes through random generator unchanged",
       [](const GenParams &params) {
         const auto gen = gen::resize(0, genPassedParams());
         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(value.random == params.random);
       });
}

TEST_CASE("gen::scale") {
  prop("scales the size by the specified factor",
       [](const GenParams &params) {
         const auto gen = gen::scale(2.0, genPassedParams());
         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(value.size == params.size * 2);
       });

  prop("passes through random generator unchanged",
       [](const GenParams &params) {
         const auto gen = gen::scale(2.0, genPassedParams());
         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(value.random == params.random);
       });
}

TEST_CASE("gen::noShrink") {
  prop("returned shrinkable has expected value",
       [](const GenParams &params, int x) {
         const auto gen = gen::noShrink(gen::just(x));
         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(value == x);
       });

  prop("returned shrinkable has no shrinks",
       [](const GenParams &params) {
         const auto gen = gen::noShrink(gen::arbitrary<int>());
         const auto shrinkable = gen(params.random, params.size);
         RC_ASSERT(!shrinkable.shrinks().next());
       });

  prop("passes generation params unchanged",
       [](const GenParams &params) {
         const auto gen = gen::noShrink(genPassedParams());
         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(value == params);
       });
}

TEST_CASE("gen::withSize") {
  prop("passes the current size to the callable",
       [](const GenParams &params) {
         const auto gen =
             gen::withSize([](int size) { return gen::just(size); });
         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(value == params.size);
       });

  prop("generates what the returned generator generates",
       [](const GenParams &params, int x) {
         const auto gen = gen::withSize([=](int size) { return gen::just(x); });
         const auto shrinkable = gen(params.random, params.size);
         RC_ASSERT(shrinkable == shrinkable::just(x));
       });

  prop("passes generation params unchanged",
       [](const GenParams &params) {
         const auto gen =
             gen::withSize([](int size) { return genPassedParams(); });
         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(value == params);
       });
}
