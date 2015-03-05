#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/shrinkable/Operations.h"
#include "rapidcheck/shrinkable/Create.h"

using namespace rc;

TEST_CASE("shrinkable::all") {
    prop("returns true if predicate returns true for all",
         [] {
             // TODO sized ranged
             int x = *gen::ranged<int>(1, 5);
             const auto shrinkable = shrinkable::shrinkRecur(x, [](int x) {
                 return seq::range(x - 1, 0);
             });

             RC_ASSERT(shrinkable::all(shrinkable,
                                       [](const Shrinkable<int> &s) {
                                           return s.value() != 0;
                                       }));
         });

    prop("returns true if predicate returns false for at least one shrinkable",
         [] {
             // TODO sized ranged
             int x = *gen::ranged<int>(0, 5);
             const auto shrinkable = shrinkable::just(
                 x, seq::map(&shrinkable::just<int>, seq::range(x - 1, -1)));

             RC_ASSERT(!shrinkable::all( shrinkable,
                                         [](const Shrinkable<int> &s) {
                                             return s.value() != 0;
                                         }));
         });
}
