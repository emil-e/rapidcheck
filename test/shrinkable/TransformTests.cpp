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
    prop(
        "maps value()",
        [](int x) {
            const auto shrinkable = shrinkable::map(
                shrinkable::just(x),
                doubleIt);
            RC_ASSERT(shrinkable.value() == doubleIt(x));
        });

    prop(
        "maps shrinks()",
        [](Shrinkable<int> from) {
            const auto shrinkable = shrinkable::map(from, doubleIt);
            const auto expected = seq::map(
                from.shrinks(),
                [](Shrinkable<int> &&shrink) {
                    return shrinkable::map(shrink, doubleIt);
                });

            RC_ASSERT(shrinkable.shrinks() == expected);
        });
}

TEST_CASE("shrinkable::mapShrinks") {
    prop(
        "maps shrinks with the given mapping callable",
        [](Shrinkable<int> shrinkable) {
            const auto mapper = [](Seq<Shrinkable<int>> &&shrinkable) {
                return seq::map(
                    std::move(shrinkable),
                    [](const Shrinkable<int> &shrink) {
                        return shrinkable::just(shrink.value());
                    });
            };

            const auto mapped = shrinkable::mapShrinks(shrinkable, mapper);
            RC_ASSERT(mapped.shrinks() == mapper(shrinkable.shrinks()));
        });

    prop(
        "leaves value unaffected",
        [](int x) {
            const auto shrinkable = shrinkable::mapShrinks(
                shrinkable::just(x),
                fn::constant(Seq<Shrinkable<int>>()));
            RC_ASSERT(shrinkable.value() == x);
        });
}

TEST_CASE("shrinkable::filter") {
    prop(
        "returns Nothing if predicate returns false for value",
        [](int x) {
            const auto shrinkable = shrinkable::just(x);
            RC_ASSERT(!shrinkable::filter(shrinkable, fn::constant(false)));
        });

    prop(
        "returned shrinkable does not contain any value for which predicate"
        " returns false",
        [] {
            const auto pred = [](int x) { return (x % 2) == 0; };
            const auto shrinkable = *gen::suchThat<Shrinkable<int>>(
                [&](const Shrinkable<int> &x) {
                    return pred(x.value());
                });
            const auto filtered = *shrinkable::filter(shrinkable, pred);
            RC_ASSERT(shrinkable::all(filtered, [&](const Shrinkable<int> &s) {
                return pred(s.value());
            }));
        });

    prop(
        "if the filter returns true for every value, the returned shrinkable"
        " is equal to the original",
        [](Shrinkable<int> shrinkable) {
            RC_ASSERT(*shrinkable::filter(shrinkable, fn::constant(true)) ==
                      shrinkable);
        });
}

TEST_CASE("shrinkable::pair") {
    prop(
        "has no shrinks if the two Shrinkables have no shrinks",
        [](int a, int b) {
            REQUIRE(shrinkable::pair(shrinkable::just(a),
                                     shrinkable::just(b)) ==
                    shrinkable::just(std::make_pair(a, b)));
        });

    prop(
        "shrinks first element and then the second one",
        [] {
            const auto a = *gen::inRange<int>(0, 3);
            const auto b = *gen::inRange<int>(0, 3);
            const auto makeShrinkable = [](int x) {
                return shrinkable::shrinkRecur(x, [](int v) {
                    return seq::range(v - 1, -1);
                });
            };
            const auto shrinkable = shrinkable::pair(
                makeShrinkable(a),
                makeShrinkable(b));
            const auto expected = shrinkable::shrinkRecur(
                std::make_pair(a, b), [](const std::pair<int, int> &p) {
                    return seq::concat(
                        seq::map(seq::range(p.first - 1, -1), [=](int x) {
                            return std::make_pair(x, p.second);
                        }),
                        seq::map(seq::range(p.second - 1, -1), [=](int x) {
                            return std::make_pair(p.first, x);
                        }));
                });

            RC_ASSERT(shrinkable == expected);
        });
}
