#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/Predictable.h"

using namespace rc;
using namespace rc::detail;

TEST_CASE("gen::distinctFrom") {
    prop("never generates a value that is equal to the given one",
         [] (int x) {
             RC_ASSERT(*gen::distinctFrom(x) != x);
         });

    prop("uses the correct generator when specified",
         [] (int x) {
             RC_ASSERT(*gen::distinctFrom(gen::constant(x), x - 1) == x);
         });

    prop("uses the correct arbitrary instance",
         [] (const Predictable &x) {
             RC_ASSERT(isArbitraryPredictable(*gen::distinctFrom(x)));
         });
}
