#include <catch.hpp>
#include <rapidcheck/catch.h>

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

TEST_CASE("gen::mapcat") {
  // It would be nice and all to test the monad laws here but since `Gen` is
  // only morally a monad because of random splitting, it's simpler this way
  prop("mapcats the returned shrinkables",
       [](Shrinkable<int> a, Shrinkable<int> b) {
         const auto expected = shrinkable::mapcat(
             a,
             [=](int x) {
               return shrinkable::map(
                   b, [=](int y) { return std::make_pair(x, y); });
             });
         const auto gen = gen::mapcat<int>(
             fn::constant(a),
             [=](int x) -> Gen<std::pair<int, int>> {
               return fn::constant(shrinkable::map(
                   b, [=](int y) { return std::make_pair(x, y); }));
             });
         const auto actual = gen(Random(), 0);

         RC_ASSERT(actual == expected);
       });

  prop("passes correct size",
       [](const GenParams &params) {
         const auto gen = gen::mapcat(
             genSize(),
             [](int x) {
               return gen::map(genSize(),
                               [=](int y) { return std::make_pair(x, y); });
             });

         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(value == std::make_pair(params.size, params.size));
       });

  prop("passes unique random generators",
       [](const GenParams &params) {
         const auto gen = gen::mapcat(
             genRandom(),
             [](const Random &x) {
               return gen::map(
                   genRandom(),
                   [=](Random &&y) { return std::make_pair(x, std::move(y)); });
             });

         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(value.first != value.second);
         RC_ASSERT(value.first != params.random);
         RC_ASSERT(value.second != params.random);
       });

  SECTION("works with non-copyable types") {
    const auto gen = gen::mapcat(gen::arbitrary<NonCopyable>(),
                                 [](NonCopyable &&x) {
                                   RC_ASSERT(isArbitraryPredictable(x));
                                   return gen::arbitrary<NonCopyable>();
                                 });
    const auto value = gen(Random(), 0).value();
    RC_ASSERT(isArbitraryPredictable(value));
  }
}

TEST_CASE("gen::join") {
  prop("gen::join(gen::map(s, f)) == gen::mapcat(s, f)",
       [](Shrinkable<int> a, Shrinkable<int> b) {
         const auto f = [=](int x) -> Gen<std::pair<int, int>> {
           return fn::constant(
               shrinkable::map(b, [=](int y) { return std::make_pair(x, y); }));
         };
         const auto expected =
             gen::mapcat(Gen<int>(fn::constant(a)), f)(Random(), 0);
         const auto actual =
             gen::join(gen::map(Gen<int>(fn::constant(a)), f))(Random(), 0);

         RC_ASSERT(actual == expected);
       });

  prop("passes correct size",
       [](const GenParams &params) {
         const auto gen = gen::join(gen::map(
             genSize(),
             [=](int x) {
               return gen::map(genSize(),
                               [=](int y) { return std::make_pair(x, y); });
             }));

         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(value == std::make_pair(params.size, params.size));
       });

  prop("passes unique random generators",
       [](const GenParams &params) {
         const auto gen =
             gen::join(gen::map(genRandom(),
                                [=](const Random &x) {
                                  return gen::map(genRandom(),
                                                  [=](const Random &y) {
                                                    return std::make_pair(x, y);
                                                  });
                                }));

         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(value.first != value.second);
         RC_ASSERT(value.first != params.random);
         RC_ASSERT(value.second != params.random);
       });

  SECTION("works with non-copyable types") {
    const auto gen = gen::join(gen::map(gen::arbitrary<NonCopyable>(),
                                        [](const NonCopyable &x) {
                                          RC_ASSERT(isArbitraryPredictable(x));
                                          return gen::arbitrary<NonCopyable>();
                                        }));

    const auto value = gen(Random(), 0).value();
    RC_ASSERT(isArbitraryPredictable(value));
  }
}

TEST_CASE("gen::apply") {
  prop("has tuple shrinking semantics",
       [] {
         const auto g1 = genFixedCountdown(*gen::inRange(0, 10));
         const auto g2 = genFixedCountdown(*gen::inRange(0, 10));
         const auto g3 = genFixedCountdown(*gen::inRange(0, 10));

         const auto gen = gen::apply([](int a, int b, int c) {
           return std::make_tuple(a, b, c);
         }, g1, g2, g3);
         const auto tupleGen = gen::tuple(g1, g2, g3);

         assertEquivalent(gen(Random(), 0), tupleGen(Random(), 0));
       });

  prop("passes correct size",
       [](const GenParams &params) {
         const auto gen = gen::apply([](int a, int b, int c) {
           return std::make_tuple(a, b, c);
         }, genSize(), genSize(), genSize());
         const auto value = gen(params.random, params.size).value();

         RC_ASSERT(value ==
                   std::make_tuple(params.size, params.size, params.size));
       });

  prop("passed random generators are unique",
       [](const GenParams &params) {
         const auto gen = gen::apply([](Random &&a, Random &&b, Random &&c) {
           return std::make_tuple(std::move(a), std::move(b), std::move(c));
         }, genRandom(), genRandom(), genRandom());
         const auto value = gen(params.random, params.size).value();

         RC_ASSERT(std::get<0>(value) != std::get<1>(value));
         RC_ASSERT(std::get<0>(value) != std::get<2>(value));
         RC_ASSERT(std::get<1>(value) != std::get<2>(value));
       });

  SECTION("works with non-copyable types") {
    const auto gen = gen::apply([](NonCopyable &&a, NonCopyable &&b) {
      return std::make_tuple(std::move(a), std::move(b));
    }, gen::arbitrary<NonCopyable>(), gen::arbitrary<NonCopyable>());
    const auto value = gen(Random(), 0).value();

    RC_ASSERT(isArbitraryPredictable(std::get<0>(value)));
    RC_ASSERT(isArbitraryPredictable(std::get<1>(value)));
  }
}

TEST_CASE("gen::cast") {
  prop("casting to a larger type and then back yields original",
       [](const Shrinkable<uint8_t> &shrinkable) {
         const Gen<uint8_t> gen(fn::constant(shrinkable));
         const auto cast = gen::cast<uint8_t>(gen::cast<int>(gen));
         RC_ASSERT(cast(Random(), 0) == shrinkable);
       });
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

TEST_CASE("gen::shrink") {
  prop("passes generation params unchanged",
       [](const GenParams &params) {
         const auto gen =
             gen::shrink(genPassedParams(), fn::constant(Seq<GenParams>()));
         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(value == params);
       });

  prop("applies postShrink to returned Shrinkable",
       [](const GenParams &params) {
         const auto gen = gen::arbitrary<int>();
         const auto f = [](int v) {
           return seq::takeWhile(seq::iterate(v, [](int x) { return x / 2; }),
                                 [](int x) { return x > 0; });
         };
         const auto expected =
             shrinkable::postShrink(gen(params.random, params.size), f);
         const auto actual = gen::shrink(gen, f)(params.random, params.size);

         assertEquivalent(actual, expected);
       });
}
