#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/shrinkable/Transform.h"
#include "rapidcheck/shrinkable/Create.h"

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

