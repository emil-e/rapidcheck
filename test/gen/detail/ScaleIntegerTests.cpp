#include <catch.hpp>
#include <rapidcheck-catch.h>

using namespace rc;

TEST_CASE("scaleInteger") {
  prop("yields input for kNominalSize",
       [](uint64_t x) {
         RC_ASSERT(gen::detail::scaleInteger(x, kNominalSize) == x);
       });

  prop("yields 0 for 0",
       [](uint64_t x) { RC_ASSERT(gen::detail::scaleInteger(x, 0) == 0); });
}
