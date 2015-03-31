#include <catch.hpp>
#include <rapidcheck-catch.h>

#include <numeric>

#include "util/Predictable.h"
#include "util/GenUtils.h"
#include "util/ArbitraryRandom.h"

#include "rapidcheck/newgen/detail/ExecRaw.h"
#include "rapidcheck/shrinkable/Operations.h"
#include "rapidcheck/seq/Operations.h"

using namespace rc;
using namespace rc::test;
using namespace rc::newgen::detail;

namespace {

template<int N>
Gen<std::pair<std::vector<int>, Recipe>> testExecGen()
{
    return execRaw(
        [=](const FixedCountdown<N> &n) {
            std::vector<int> values;
            values.push_back(n.value);
            while (values.size() < (n.value + 1))
                values.push_back(*genFixedCountdown(N));
            return values;
        });
}

} // namespace

TEST_CASE("execRaw") {
    SECTION("uses correct arbitrary instance for arguments") {
        auto values = execRaw([](
            const Predictable &a,
            const Predictable &b,
            const Predictable &c)
        {
            return std::make_tuple(a, b, c);
        })(Random(), 0).value().first;

        REQUIRE(isArbitraryPredictable(std::get<0>(values)));
        REQUIRE(isArbitraryPredictable(std::get<1>(values)));
        REQUIRE(isArbitraryPredictable(std::get<2>(values)));
    }

    SECTION("shrinks arguments like a tuple") {
        typedef std::tuple<FixedCountdown<1>,
                           FixedCountdown<2>,
                           FixedCountdown<3>> TupleT;

        auto execShrinkable = execRaw([](
            const FixedCountdown<1> &a,
            const FixedCountdown<2> &b,
            const FixedCountdown<3> &c)
        {
            return std::make_tuple(a, b, c);
        })(Random(), 0);

        auto tupleShrinkable = newgen::tuple(
            newgen::arbitrary<FixedCountdown<1>>(),
            newgen::arbitrary<FixedCountdown<2>>(),
            newgen::arbitrary<FixedCountdown<3>>())(Random(), 0);

        auto mappedShrinkable = shrinkable::map(
            execShrinkable,
            [](std::pair<TupleT, Recipe> &&x) {
                return std::move(x.first);
            });

        REQUIRE(mappedShrinkable == tupleShrinkable);
    }

    prop("traversing a random path through the shrink tree yields the expected"
         " values",
         []{
             const auto shrinkable = testExecGen<5>()(Random(), 0);
             const auto path = *gen::collection<std::vector<bool>>(
                 gen::arbitrary<bool>());

             std::vector<int> accepted = shrinkable.value().first;
             auto acceptedShrinkable = shrinkable;
             auto shrinks = shrinkable.shrinks();
             int i = 0;
             int x = 5;
             for (bool accept : path) {
                 if (i >= accepted.size()) {
                     RC_ASSERT(!shrinks.next());
                     break;
                 }

                 auto shrink = *shrinks.next();
                 auto actual = shrink.value().first;

                 auto expected = accepted;
                 expected[i] = --x;
                 // First element determines the size
                 expected.resize(expected[0] + 1);
                 RC_ASSERT(actual == expected);
                 if (accept) {
                     accepted = expected;
                     acceptedShrinkable = shrink;
                     shrinks = shrink.shrinks();
                 }

                 if (x == 0) {
                     x = 5;
                     i++;
                 }
             }
         });

    prop("the number of shrinks is never never more than the sum of the number"
         " of shrinks for the requested values",
         []{
             const auto shrinkable = testExecGen<5>()(Random(), 0);

             int i = *gen::ranged<int>(0, 5);
             const auto shrink = *seq::at(shrinkable.shrinks(), i);
             const auto value = shrink.value().first;
             int maxShrinks =
                 std::accumulate(begin(value), end(value), 0);
             RC_ASSERT(seq::length(shrink.shrinks()) <= maxShrinks);
         });

    prop("passes on the correct size",
         [] {
             int expectedSize = *gen::nonNegative<int>();
             int n = *gen::ranged<int>(1, 10);
             auto shrinkable = execRaw([=](const PassedSize &sz) {
                 *genFixedCountdown(3); // Force some shrinks
                 std::vector<int> sizes;
                 sizes.push_back(sz.value);
                 while (sizes.size() < n)
                     sizes.push_back(*genSize());
                 return sizes;
             })(Random(), expectedSize);

             auto valueShrinkable = shrinkable::map(
                 shrinkable,
                 [](std::pair<std::vector<int>, Recipe> &&x) {
                     return std::move(x.first);
                 });

             RC_ASSERT(
                 shrinkable::all(
                     valueShrinkable,
                     [=](const Shrinkable<std::vector<int>> &x) {
                         auto sizes = x.value();
                         return std::all_of(
                             begin(sizes), end(sizes),
                             [=](int sz) { return sz == expectedSize; });
                     }));
         });

    prop("passes on sequentially split generators",
         [](const Random &initial) {
             int n = *gen::ranged<int>(1, 10);
             auto shrinkable = execRaw([=](const PassedRandom &rnd) {
                 std::vector<Random> randoms;
                 randoms.push_back(rnd.value);
                 while (randoms.size() < n)
                     randoms.push_back(*genRandom());
                 // Force some shrinks, must be last because it will steal a
                 // random split otherwise
                 *genFixedCountdown(3);
                 return randoms;
             })(initial, 0);

             Random r(initial);
             std::vector<Random> expected;
             // The arguments are generated as a tuple so we want the same
             // splitting behavior
             expected.push_back(
                 std::get<0>(newgen::tuple(genRandom())(r.split(), 0).value()));
             while (expected.size() < n)
                 expected.push_back(r.split());

             auto valueShrinkable = shrinkable::map(
                 shrinkable,
                 [](std::pair<std::vector<Random>, Recipe> &&x) {
                     return std::move(x.first);
                 });

             RC_ASSERT(
                 shrinkable::all(
                     valueShrinkable,
                     [=](const Shrinkable<std::vector<Random>> &x) {
                         return x.value() == expected;
                     }));
         });

    SECTION("the ingredients of the recipe exactly match the generated value") {
        REQUIRE(
            shrinkable::all(
                testExecGen<3>()(Random(), 0),
                [](const Shrinkable<std::pair<std::vector<int>, Recipe>> &x) {
                    typedef std::tuple<FixedCountdown<3>> ArgTuple;
                    const auto pair = x.value();
                    const auto recipe = pair.second;

                    std::vector<int> actual;

                    auto argTuple =
                        recipe.ingredients.front().value().get<ArgTuple>();
                    actual.push_back(std::get<0>(argTuple).value);

                    auto it = recipe.ingredients.begin() + 1;
                    for (; it != end(recipe.ingredients); it++)
                        actual.push_back(it->value().get<int>());

                    return actual == pair.first;
                }));
    }

    // TODO test non-copyables!
}
