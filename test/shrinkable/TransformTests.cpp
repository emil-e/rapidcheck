#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/shrinkable/Transform.h"
#include "rapidcheck/shrinkable/Create.h"
#include "rapidcheck/shrinkable/Operations.h"

#include "util/Generators.h"
#include "util/CopyGuard.h"
#include "util/ShrinkableUtils.h"

using namespace rc;
using namespace rc::test;

int doubleIt(int x) { return x * 2; }

TEST_CASE("shrinkable::map") {
  prop("maps value()",
       [](int x) {
         const auto shrinkable = shrinkable::map(shrinkable::just(x), doubleIt);
         RC_ASSERT(shrinkable.value() == doubleIt(x));
       });

  prop("maps shrinks()",
       [](Shrinkable<int> from) {
         const auto shrinkable = shrinkable::map(from, doubleIt);
         const auto expected =
             seq::map(from.shrinks(),
                      [](Shrinkable<int> &&shrink) {
                        return shrinkable::map(shrink, doubleIt);
                      });

         RC_ASSERT(shrinkable.shrinks() == expected);
       });
}

TEST_CASE("shrinkable::mapShrinks") {
  prop("maps shrinks with the given mapping callable",
       [](Shrinkable<int> shrinkable) {
         const auto mapper = [](Seq<Shrinkable<int>> &&shrinkable) {
           return seq::map(std::move(shrinkable),
                           [](const Shrinkable<int> &shrink) {
                             return shrinkable::just(shrink.value());
                           });
         };

         const auto mapped = shrinkable::mapShrinks(shrinkable, mapper);
         RC_ASSERT(mapped.shrinks() == mapper(shrinkable.shrinks()));
       });

  prop("leaves value unaffected",
       [](int x) {
         const auto shrinkable = shrinkable::mapShrinks(
             shrinkable::just(x), fn::constant(Seq<Shrinkable<int>>()));
         RC_ASSERT(shrinkable.value() == x);
       });
}

TEST_CASE("shrinkable::filter") {
  prop("returns Nothing if predicate returns false for value",
       [](int x) {
         const auto shrinkable = shrinkable::just(x);
         RC_ASSERT(!shrinkable::filter(shrinkable, fn::constant(false)));
       });

  prop(
      "returned shrinkable does not contain any value for which predicate"
      " returns false",
      [] {
        const auto pred = [](int x) { return (x % 2) == 0; };
        const auto shrinkable = *gen::suchThat<Shrinkable<int>>([&](
            const Shrinkable<int> &x) { return pred(x.value()); });
        const auto filtered = *shrinkable::filter(shrinkable, pred);
        RC_ASSERT(shrinkable::all(
            filtered,
            [&](const Shrinkable<int> &s) { return pred(s.value()); }));
      });

  prop(
      "if the filter returns true for every value, the returned shrinkable"
      " is equal to the original",
      [](Shrinkable<int> shrinkable) {
        RC_ASSERT(*shrinkable::filter(shrinkable, fn::constant(true)) ==
                  shrinkable);
      });
}

TEST_CASE("shrinkable::mapcat") {
  SECTION("monad laws") {
    prop("left identity",
         [] {
           const auto a = *gen::inRange(0, 10);
           const auto s =
               shrinkable::mapcat(shrinkable::just(a), &countdownShrinkable);
           RC_ASSERT(s == countdownShrinkable(a));
         });

    prop("right identity",
         [](Shrinkable<int> m) {
           const auto s = shrinkable::mapcat(
               m, [](int v) { return shrinkable::just(v); });
           RC_ASSERT(s == m);
         });

    prop("associativity",
         [](Shrinkable<int> m) {
           const auto f = &countdownShrinkable;
           const auto g = [](int x) {
             return shrinkable::map(countdownShrinkable(x),
                                    [](int v) { return v / 2; });
           };

           const auto s1 = shrinkable::mapcat(shrinkable::mapcat(m, f), g);
           const auto s2 = shrinkable::mapcat(
               m, [=](int x) { return shrinkable::mapcat(f(x), g); });
           assertEquivalent(s1, s2);
         });
  }

  static const auto pairShrinkable = [](int a, int b) {
    return shrinkable::mapcat(
        countdownShrinkable(a),
        [=](int x) {
          return shrinkable::map(countdownShrinkable(b),
                                 [=](int y) { return std::make_pair(x, y); });
        });
  };

  prop("first tries to shrink first shrinkable",
       [] {
         const auto small = gen::inRange(0, 10);
         const auto xs = *gen::pair(small, small);
         const auto shrinkable = pairShrinkable(xs.first, xs.second);

         const auto shrinks = seq::map(
             seq::take(xs.first, shrinkable::immediateShrinks(shrinkable)),
             [](const std::pair<int, int> &p) { return p.first; });

         RC_ASSERT(shrinks == countdownSeq(xs.first));
       });

  prop("tries to shrink second shrinkable after first shrinkable",
       [] {
         const auto small = gen::inRange(0, 10);
         const auto xs = *gen::pair(small, small);
         const auto shrinkable = pairShrinkable(xs.first, xs.second);

         const auto shrinks = seq::map(
             seq::drop(xs.first, shrinkable::immediateShrinks(shrinkable)),
             [](const std::pair<int, int> &p) { return p.second; });

         RC_ASSERT(shrinks == countdownSeq(xs.second));
       });

  prop("when trying to shrink the second shrinkable, the first one is fixed",
       [] {
         const auto small = gen::inRange(0, 10);
         const auto xs = *gen::pair(small, small);
         const auto shrinkable = pairShrinkable(xs.first, xs.second);

         const auto firstValue = shrinkable.value().first;
         const auto n = *gen::inRange(xs.first, xs.first + xs.second);
         const auto shrink = *seq::at(shrinkable.shrinks(), n);
         onAnyPath(shrink,
                   [=](const Shrinkable<std::pair<int, int>> &value,
                       const Shrinkable<std::pair<int, int>> &shrink) {
                     RC_ASSERT(value.value().first == firstValue);
                   });
       });
}

TEST_CASE("shrinkable::pair") {
  prop("has no shrinks if the two Shrinkables have no shrinks",
       [](int a, int b) {
         REQUIRE(shrinkable::pair(shrinkable::just(a), shrinkable::just(b)) ==
                 shrinkable::just(std::make_pair(a, b)));
       });

  prop("shrinks first element and then the second one",
       [] {
         const auto a = *gen::inRange<int>(0, 3);
         const auto b = *gen::inRange<int>(0, 3);
         const auto makeShrinkable = [](int x) {
           return shrinkable::shrinkRecur(
               x, [](int v) { return seq::range(v - 1, -1); });
         };
         const auto shrinkable =
             shrinkable::pair(makeShrinkable(a), makeShrinkable(b));
         const auto expected = shrinkable::shrinkRecur(
             std::make_pair(a, b),
             [](const std::pair<int, int> &p) {
               return seq::concat(
                   seq::map(seq::range(p.first - 1, -1),
                            [=](int x) { return std::make_pair(x, p.second); }),
                   seq::map(seq::range(p.second - 1, -1),
                            [=](int x) { return std::make_pair(p.first, x); }));
             });

         RC_ASSERT(shrinkable == expected);
       });
}
