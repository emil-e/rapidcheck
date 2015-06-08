#include <catch.hpp>
#include <rapidcheck-catch.h>

#include <numeric>

#include "util/Predictable.h"
#include "util/GenUtils.h"
#include "util/ArbitraryRandom.h"

#include "rapidcheck/gen/detail/ExecRaw.h"
#include "rapidcheck/shrinkable/Operations.h"
#include "rapidcheck/seq/Operations.h"

using namespace rc;
using namespace rc::test;
using namespace rc::gen::detail;

namespace {

template <int N>
Gen<std::pair<std::vector<int>, Recipe>> testExecGen() {
  return execRaw([=](const FixedCountdown<N> &n) {
    std::vector<int> values;
    values.push_back(n.value);
    while (values.size() < (n.value + 1)) {
      values.push_back(*genFixedCountdown(N));
    }
    return values;
  });
}

} // namespace

TEST_CASE("execRaw") {
  SECTION("uses correct arbitrary instance for arguments") {
    const auto values = execRaw([](
        const Predictable &a, const Predictable &b, const Predictable &c) {
      return std::make_tuple(a, b, c);
    })(Random(), 0).value().first;

    REQUIRE(isArbitraryPredictable(std::get<0>(values)));
    REQUIRE(isArbitraryPredictable(std::get<1>(values)));
    REQUIRE(isArbitraryPredictable(std::get<2>(values)));
  }

  SECTION("shrinks arguments like a tuple") {
    using TupleT =
        std::tuple<FixedCountdown<1>, FixedCountdown<2>, FixedCountdown<3>>;

    const auto execShrinkable = execRaw([](const FixedCountdown<1> &a,
                                           const FixedCountdown<2> &b,
                                           const FixedCountdown<3> &c) {
      return std::make_tuple(a, b, c);
    })(Random(), 0);

    const auto tupleShrinkable =
        gen::tuple(gen::arbitrary<FixedCountdown<1>>(),
                   gen::arbitrary<FixedCountdown<2>>(),
                   gen::arbitrary<FixedCountdown<3>>())(Random(), 0);

    const auto mappedShrinkable = shrinkable::map(
        execShrinkable,
        [](std::pair<TupleT, Recipe> &&x) { return std::move(x.first); });

    REQUIRE(mappedShrinkable == tupleShrinkable);
  }

  prop(
      "traversing a random path through the shrink tree yields the expected"
      " values",
      [] {
        const auto shrinkable = testExecGen<5>()(Random(), 0);
        const auto path =
            *gen::container<std::vector<bool>>(gen::arbitrary<bool>());

        auto accepted = shrinkable.value().first;
        auto acceptedShrinkable = shrinkable;
        auto shrinks = shrinkable.shrinks();
        auto i = 0;
        auto x = 5;
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

  prop(
      "the number of shrinks is never never more than the sum of the number"
      " of shrinks for the requested values",
      [] {
        const auto shrinkable = testExecGen<5>()(Random(), 0);

        const auto i = *gen::inRange<int>(0, 5);
        const auto shrink = *seq::at(shrinkable.shrinks(), i);
        const auto value = shrink.value().first;
        const auto maxShrinks = std::accumulate(begin(value), end(value), 0);
        RC_ASSERT(seq::length(shrink.shrinks()) <= maxShrinks);
      });

  prop("passes on the correct size",
       [] {
         const auto expectedSize = *gen::nonNegative<int>();
         const auto n = *gen::inRange<int>(1, 10);
         const auto shrinkable = execRaw([=](const PassedSize &sz) {
           *genFixedCountdown(3); // Force some shrinks
           std::vector<int> sizes;
           sizes.push_back(sz.value);
           while (sizes.size() < n) {
             sizes.push_back(*genSize());
           }
           return sizes;
         })(Random(), expectedSize);

         auto valueShrinkable =
             shrinkable::map(shrinkable,
                             [](std::pair<std::vector<int>, Recipe> &&x) {
                               return std::move(x.first);
                             });

         RC_ASSERT(shrinkable::all(
             valueShrinkable,
             [=](const Shrinkable<std::vector<int>> &x) {
               auto sizes = x.value();
               return std::all_of(begin(sizes),
                                  end(sizes),
                                  [=](int sz) { return sz == expectedSize; });
             }));
       });

  prop("passed generators are unique",
       [](const Random &initial) {
         const auto n = *gen::inRange<int>(1, 10);
         const auto randoms = execRaw([=](const PassedRandom &rnd) {
           std::set<Random> randoms;
           randoms.insert(rnd.value);
           while (randoms.size() < n) {
             randoms.insert(*genRandom());
           }
           return randoms;
         })(initial, 0).value().first;

         RC_ASSERT(randoms.size() == n);
       });

  prop("passed randoms do not change with shrinking",
       [](const Random &initial) {
         const auto n = *gen::inRange<int>(1, 10);
         const auto shrinkable = execRaw([=](const PassedRandom &rnd) {
           std::vector<Random> randoms;
           randoms.push_back(rnd.value);
           while (randoms.size() < n) {
             randoms.push_back(*genRandom());
           }
           *genFixedCountdown(3);
           return randoms;
         })(initial, 0);

         const auto valueShrinkable =
             shrinkable::map(shrinkable,
                             [](std::pair<std::vector<Random>, Recipe> &&x) {
                               return std::move(x.first);
                             });

         const auto head = valueShrinkable.value();
         RC_ASSERT(
             shrinkable::all(valueShrinkable,
                             [=](const Shrinkable<std::vector<Random>> &x) {
                               return x.value() == head;
                             }));
       });

  SECTION("the ingredients of the recipe exactly match the generated value") {
    REQUIRE(shrinkable::all(
        testExecGen<3>()(Random(), 0),
        [](const Shrinkable<std::pair<std::vector<int>, Recipe>> &x) {
          using ArgTuple = std::tuple<FixedCountdown<3>>;
          const auto pair = x.value();
          const auto recipe = pair.second;

          std::vector<int> actual;

          const auto argTuple =
              recipe.ingredients.front().value().get<ArgTuple>();
          actual.push_back(std::get<0>(argTuple).value);

          auto it = recipe.ingredients.begin() + 1;
          for (; it != end(recipe.ingredients); it++) {
            actual.push_back(it->value().get<int>());
          }

          return actual == pair.first;
        }));
  }

  SECTION("works with non-copyable types") {
    const auto shrinkable =
        execRaw([=](NonCopyable nc) { return std::move(nc); })(Random(), 0);
    REQUIRE(isArbitraryPredictable(shrinkable.value().first));
  }

  SECTION("empty arguments don't show up in tuple") {
    const auto value = execRaw([] { return 0; })(Random(), 0).value();
    REQUIRE(value.second.ingredients.empty());
  }

  prop("disallows nested use of operator*",
       [](const GenParams &params) {
         const auto gen = execRaw([] {
           return *Gen<int>([](const Random &, int) {
             *gen::just(1337);
             return shrinkable::just(1337);
           });
         });
         const auto shrinkable = gen(params.random, params.size);
         // TODO RC_ASSERT_THROWS
         try {
           shrinkable.value();
         } catch (...) {
           RC_SUCCEED("Threw exception");
         }
         RC_FAIL("Did not throw exception");
       });
}
