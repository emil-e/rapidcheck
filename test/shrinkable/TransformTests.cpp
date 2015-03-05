#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/shrinkable/Transform.h"
#include "rapidcheck/shrinkable/Create.h"
#include "rapidcheck/shrinkable/Operations.h"

#include "util/Generators.h"
#include "util/CopyGuard.h"

using namespace rc;
using namespace rc::test;

int doubleIt(int x) { return x * 2; }

TEST_CASE("shrinkable::map") {
    prop("maps value()",
         [](int x) {
             const auto shrinkable = shrinkable::map(
                 doubleIt, shrinkable::just(x));
             RC_ASSERT(shrinkable.value() == doubleIt(x));
         });

    prop("maps shrinks()",
         [](Shrinkable<int> from) {
             const auto shrinkable = shrinkable::map(doubleIt, from);
             const auto expected = seq::map([](Shrinkable<int> &&shrink) {
                 return shrinkable::map(doubleIt, shrink);
             }, from.shrinks());

             RC_ASSERT(shrinkable.shrinks() == expected);
         });

    prop("does not copy shrinkable on construction",
         [](Shrinkable<CopyGuard> from) {
             const auto shrinkable = shrinkable::map(
                 [](CopyGuard &&x) { return std::move(x); },
                 std::move(from));
         });
}

TEST_CASE("shrinkable::mapShrinks") {
    prop("maps shrinks with the given mapping callable",
         [](Shrinkable<int> shrinkable) {
             const auto mapper = [](Seq<Shrinkable<int>> &&shrinkable) {
                 return seq::map([](const Shrinkable<int> &shrink) {
                     return shrinkable::just(shrink.value());
                 }, std::move(shrinkable));
             };

             const auto mapped = shrinkable::mapShrinks(mapper, shrinkable);
             RC_ASSERT(mapped.shrinks() == mapper(shrinkable.shrinks()));
         });

    prop("leaves value unaffected",
         [](int x) {
             const auto shrinkable = shrinkable::mapShrinks(
                 fn::constant(Seq<Shrinkable<int>>()),
                 shrinkable::just(x));
             RC_ASSERT(shrinkable.value() == x);
         });

    prop("does not copy shrinkable on construction",
         [](Shrinkable<CopyGuard> from) {
             const auto shrinkable = shrinkable::mapShrinks(
                 fn::constant(Seq<Shrinkable<CopyGuard>>()),
                 std::move(from));
         });
}

TEST_CASE("shrinkable::filter") {
    prop("returns Nothing if predicate returns false for value",
         [](int x) {
             const auto shrinkable = shrinkable::just(x);
             RC_ASSERT(!shrinkable::filter(fn::constant(false), shrinkable));
         });

    prop("returned shrinkable does not contain any value for which predicate"
         " returns false",
         [] {
             const auto pred = [](int x) { return (x % 2) == 0; };
             const auto shrinkable = *gen::suchThat<Shrinkable<int>>(
                 [&](const Shrinkable<int> &x) {
                     return pred(x.value());
                 });
             const auto filtered = *shrinkable::filter(pred, shrinkable);
             RC_ASSERT(shrinkable::all(filtered, [&](const Shrinkable<int> &s) {
                 return pred(s.value());
             }));
         });
}
