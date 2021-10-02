#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>

#include "util/ArbitraryRandom.h"
#include "util/Predictable.h"
#include "util/GenUtils.h"

#include "rapidcheck/gen/Tuple.h"
#include "rapidcheck/gen/Create.h"
#include "rapidcheck/shrinkable/Operations.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("gen::tuple") {
  prop(
      "the root value is a tuple of the root values from the composed"
      " generators",
      [](int x1, int x2, int x3) {
        // TODO gen::constant
        const auto gen =
            gen::tuple(gen::just(x1), gen::just(x2), gen::just(x3));
        const auto shrinkable = gen(Random(), 0);
        RC_ASSERT(shrinkable.value() == std::make_tuple(x1, x2, x3));
      });

  prop("shrinks components from left to right",
       [] {
         auto number = gen::inRange<int>(0, 3);
         const int x1 = *number;
         const int x2 = *number;
         const int x3 = *number;
         auto gen = gen::tuple(genFixedCountdown(x1),
                               genFixedCountdown(x2),
                               genFixedCountdown(x3));
         RC_ASSERT(shrinkable::all(
             gen(Random(), 0),
             [](const Shrinkable<std::tuple<int, int, int>> &s) {
               const auto value = s.value();
               auto shrinks = s.shrinks();

               for (int x = std::get<0>(value) - 1; x >= 0; x--) {
                 const auto shrink = shrinks.next();
                 const auto expected =
                     std::make_tuple(x, std::get<1>(value), std::get<2>(value));
                 if (!shrink || (shrink->value() != expected)) {
                   return false;
                 }
               }

               for (int x = std::get<1>(value) - 1; x >= 0; x--) {
                 const auto shrink = shrinks.next();
                 const auto expected =
                     std::make_tuple(std::get<0>(value), x, std::get<2>(value));
                 if (!shrink || (shrink->value() != expected)) {
                   return false;
                 }
               }

               for (int x = std::get<2>(value) - 1; x >= 0; x--) {
                 const auto shrink = shrinks.next();
                 const auto expected =
                     std::make_tuple(std::get<0>(value), std::get<1>(value), x);
                 if (!shrink || (shrink->value() != expected)) {
                   return false;
                 }
               }

               return !shrinks.next();
             }));
       });

  prop("passes the right size of each generator",
       [] {
         const int size = *gen::nonNegative<int>();
         const auto result =
             gen::tuple(genSize(), genSize(), genSize())(Random(), size);
         RC_ASSERT(result.value() == std::make_tuple(size, size, size));
       });

  prop("splits the generator N times and passes the splits from left to right",
       [](const Random &random) {
         const auto result =
             gen::tuple(genRandom(), genRandom(), genRandom())(random, 0);
         Random r(random);
         std::tuple<Random, Random, Random> expected;
         std::get<0>(expected) = r.split();
         std::get<1>(expected) = r.split();
         std::get<2>(expected) = r.split();
         RC_ASSERT(result.value() == expected);
       });

  prop("works with non-copyable types",
       [](int v1, int v2) {
         const auto xgen = gen::map(gen::just(v1),
                                    [](int x) {
                                      NonCopyable nc;
                                      nc.value = x;
                                      return nc;
                                    });
         const auto ygen = gen::map(gen::just(v2),
                                    [](int x) {
                                      NonCopyable nc;
                                      nc.value = x;
                                      return nc;
                                    });

         const auto shrinkable = gen::tuple(xgen, ygen)(Random(), 0);
         const auto result = shrinkable.value();
         RC_ASSERT(std::get<0>(result).value == v1);
         RC_ASSERT(std::get<1>(result).value == v2);
       });

  prop(
      "finds minimum if two of the values must larger than a third value"
      " which must be larger than some other value",
      [] {
        static const auto makeShrinkable = [](int x) {
          return shrinkable::shrinkRecur(
              100, [](int value) { return shrink::towards(value, 0); });
        };
        int target = *gen::inRange<int>(0, 100);
        Gen<int> gen1 = fn::constant(makeShrinkable(200));
        Gen<int> gen2 = fn::constant(makeShrinkable(100));
        Gen<int> gen3 = fn::constant(makeShrinkable(200));
        const auto shrinkable = gen::tuple(gen1, gen2, gen3)(Random(), 0);
        const auto result =
            shrinkable::findLocalMin(shrinkable,
                                     [=](const std::tuple<int, int, int> &x) {
                                       return (std::get<1>(x) >= target) &&
                                           (std::get<0>(x) > std::get<1>(x)) &&
                                           (std::get<2>(x) > std::get<1>(x));
                                     });
        RC_ASSERT(result.first ==
                  std::make_tuple(target + 1, target, target + 1));
      });
}
