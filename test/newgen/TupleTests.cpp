#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/ArbitraryRandom.h"

#include "rapidcheck/newgen/Tuple.h"
#include "rapidcheck/newgen/Create.h"
#include "rapidcheck/shrinkable/Operations.h"

using namespace rc;

Gen<int> countDownGen(int value) {
    return [=](const Random &random, int size) {
        return shrinkable::shrinkRecur(value, [](int x) {
            return seq::range(x - 1, -1);
        });
    };
};

TEST_CASE("newgen::tuple") {
    prop("the root value is a tuple of the root values from the composed"
         " generators",
         [](int x1, int x2, int x3) {
             // TODO newgen::constant
             const auto gen = newgen::tuple(newgen::just(x1),
                                      newgen::just(x2),
                                      newgen::just(x3));
             const auto shrinkable = gen(Random(), 0);
             RC_ASSERT(shrinkable.value() == std::make_tuple(x1, x2, x3));
         });

    prop("shrinks components from left to right",
         [] {
             auto number = gen::ranged<int>(0, 3);
             const int x1 = *number;
             const int x2 = *number;
             const int x3 = *number;
             auto gen = newgen::tuple(countDownGen(x1),
                                      countDownGen(x2),
                                      countDownGen(x3));
             RC_ASSERT(
                 shrinkable::all(
                     gen(Random(), 0),
                     [](const Shrinkable<std::tuple<int, int, int>> &s) {
                         const auto value = s.value();
                         auto shrinks = s.shrinks();

                         for (int x = std::get<0>(value) - 1; x >= 0; x--) {
                             const auto shrink = shrinks.next();
                             const auto expected = std::make_tuple(
                                 x, std::get<1>(value), std::get<2>(value));
                             if (!shrink || (shrink->value() != expected))
                                 return false;
                         }

                         for (int x = std::get<1>(value) - 1; x >= 0; x--) {
                             const auto shrink = shrinks.next();
                             const auto expected = std::make_tuple(
                                 std::get<0>(value), x, std::get<2>(value));
                             if (!shrink || (shrink->value() != expected))
                                 return false;
                         }

                         for (int x = std::get<2>(value) - 1; x >= 0; x--) {
                             const auto shrink = shrinks.next();
                             const auto expected = std::make_tuple(
                                 std::get<0>(value), std::get<1>(value), x);
                             if (!shrink || (shrink->value() != expected))
                                 return false;
                         }

                         return !shrinks.next();
                     }));
         });

    prop("passes the right size of each generator",
         [] {
             const int size = *gen::nonNegative<int>();
             const auto gen = Gen<int>([](const Random &random, int size) {
                 return shrinkable::just(size);
             });

             const auto result = newgen::tuple(gen, gen, gen)(Random(), size);
             RC_ASSERT(result.value() == std::make_tuple(size, size, size));
         });

    prop("splits the generator N times and passes the splits from left to right",
         [](const Random &random) {
             const auto gen = Gen<Random>([](const Random &random, int size) {
                 return shrinkable::just(random);
             });

             const auto result = newgen::tuple(gen, gen, gen)(random, 0);
             Random r(random);
             std::tuple<Random, Random, Random> expected;
             std::get<0>(expected) = r.split();
             std::get<1>(expected) = r.split();
             std::get<2>(expected) = r.split();
             RC_ASSERT(result.value() == expected);
         });
}
