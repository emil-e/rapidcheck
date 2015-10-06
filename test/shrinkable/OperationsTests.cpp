#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/shrinkable/Operations.h"
#include "rapidcheck/shrinkable/Create.h"

#include "util/Generators.h"

using namespace rc;

TEST_CASE("shrinkable::all") {
  prop("returns true if predicate returns true for all",
       [] {
         // TODO sized ranged
         int x = *gen::inRange<int>(1, 5);
         const auto shrinkable = shrinkable::shrinkRecur(
             x, [](int x) { return seq::range(x - 1, 0); });

         RC_ASSERT(shrinkable::all(
             shrinkable,
             [](const Shrinkable<int> &s) { return s.value() != 0; }));
       });

  prop("returns true if predicate returns false for at least one shrinkable",
       [] {
         // TODO sized ranged
         int x = *gen::inRange<int>(0, 5);
         const auto shrinkable = shrinkable::just(
             x, seq::map(seq::range(x - 1, -1), &shrinkable::just<int>));

         RC_ASSERT(!shrinkable::all(shrinkable,
                                    [](const Shrinkable<int> &s) {
                                      return s.value() != 0;
                                    }));
       });
}

TEST_CASE("shrinkable::findLocalMin") {
  prop("returns the original value if no value matched predicate",
       [](int x) {
         auto result =
             shrinkable::findLocalMin(shrinkable::just(x), fn::constant(true));
         RC_ASSERT(result.first == x);
       });

  prop("return zero shrinks if no value matched predicate",
       [] {
         auto result =
             shrinkable::findLocalMin(shrinkable::just(0), fn::constant(true));
         RC_ASSERT(result.second == 0);
       });

  prop(
      "searches by descending into the first value of each shrinks Seq that"
      " matches the predicate",
      [] {
        const auto expected =
            *gen::container<std::vector<int>>(gen::inRange<int>(1, 101));

        const auto pred = [&](const std::vector<int> &x) {
          int back = x.back();
          // We want to test that the first value is picked so we accept
          // both zero and the corresponding value in the expected vector
          // but since zero comes last, that will never be picked
          return (x.size() <= expected.size()) &&
              ((back == 0) || (back == expected[x.size() - 1]));
        };

        const auto shrinkable =
            shrinkable::shrinkRecur(std::vector<int>(),
                                    [](const std::vector<int> &vec) {
                                      return seq::map(seq::range<int>(100, -1),
                                                      [=](int x) {
                                                        auto shrink = vec;
                                                        shrink.push_back(x);
                                                        return shrink;
                                                      });
                                    });

        const auto result = shrinkable::findLocalMin(shrinkable, pred);
        RC_ASSERT(result.first == expected);
        RC_ASSERT(result.second == expected.size());
      });
}

TEST_CASE("shrinkable::immediateShrinks") {
  prop(
      "returns a Seq of the values of the immediate shrinks of the"
      " Shrinkable",
      [](int x, const Seq<int> &seq) {
        const auto shrinkable =
            shrinkable::just(x, seq::map(seq, &shrinkable::just<int>));
        RC_ASSERT(shrinkable::immediateShrinks(shrinkable) == seq);
      });
}

TEST_CASE("shrinkable::walkPath") {
  prop("returns the shrinkable at the end of the path",
       [] {
         const auto path = *gen::container<std::vector<std::size_t>>(
                               gen::inRange<std::size_t>(0, 100));
         const auto shrinkable =
             shrinkable::shrinkRecur(std::vector<std::size_t>(),
                                     [](const std::vector<std::size_t> &path) {
                                       return seq::map(seq::index(),
                                                       [=](std::size_t i) {
                                                         auto p = path;
                                                         p.push_back(i);
                                                         return p;
                                                       });
                                     });

         const auto result = shrinkable::walkPath(shrinkable, path);
         RC_ASSERT(result);
         RC_ASSERT(result->value() == path);
       });

  prop("returns something if path never goes outside tree",
       [] {
         const auto limit = *gen::inRange<std::size_t>(0, 100);
         const auto shrinkable = shrinkable::shrinkRecur(
             0, [=](int x) { return seq::range<std::size_t>(0, limit); });
         auto path = *gen::container<std::vector<std::size_t>>(
                         gen::inRange<std::size_t>(0, limit));

         const auto result = shrinkable::walkPath(shrinkable, path);
         RC_ASSERT(result);
       });

  prop("returns Nothing if path ever goes outside tree",
       [] {
         const auto limit = *gen::inRange<std::size_t>(0, 100);
         const auto shrinkable = shrinkable::shrinkRecur(
             0, [=](int x) { return seq::range<std::size_t>(0, limit); });
         auto path = *gen::container<std::vector<std::size_t>>(
                         gen::inRange<std::size_t>(0, limit));
         const auto i = *gen::inRange<std::size_t>(0, path.size() + 1);
         path.insert(begin(path) + i, limit);
         const auto result = shrinkable::walkPath(shrinkable, path);
         RC_ASSERT(!result);
       });
}
