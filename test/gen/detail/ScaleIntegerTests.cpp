#include <catch.hpp>
#include <rapidcheck/catch.h>

using namespace rc;

TEST_CASE("scaleInteger") {
  prop("for uint32_t, equal to naive way",
       [] {
         const uint64_t x = *gen::arbitrary<uint32_t>();
         const uint64_t size = *gen::nonNegative<int>();
         RC_ASSERT(gen::detail::scaleInteger(x, size) ==
                   ((x * std::min<uint64_t>(kNominalSize, size) +
                     (kNominalSize / 2)) /
                    kNominalSize));
       });

  prop("result strictly increases with size",
       [](uint64_t x) {
         const auto sizeA = *gen::nonNegative<int>();
         const auto sizeB = *gen::nonNegative<int>();
         const auto small = std::min(sizeA, sizeB);
         const auto large = std::max(sizeA, sizeB);

         RC_ASSERT(gen::detail::scaleInteger(x, small) <=
                   gen::detail::scaleInteger(x, large));
       });

  prop("result strictly increases with value",
       [](uint64_t a, uint64_t b){
         const auto size = *gen::nonNegative<int>();
         const auto small = std::min(a, b);
         const auto large = std::max(a, b);

         RC_ASSERT(gen::detail::scaleInteger(small, size) <=
                   gen::detail::scaleInteger(large, size));
       });

  prop("yields input for kNominalSize",
       [](uint64_t x) {
         RC_ASSERT(gen::detail::scaleInteger(x, kNominalSize) == x);
       });

  prop("yields 0 for 0",
       [](uint64_t x) { RC_ASSERT(gen::detail::scaleInteger(x, 0) == 0); });
}
