#include <catch.hpp>
#include <rapidcheck/catch.h>

#include <numeric>

#include "rapidcheck/detail/FrequencyMap.h"

using namespace rc;
using namespace rc::detail;

TEST_CASE("FrequencyMap") {
  static const auto genFrequencies = gen::nonEmpty(
      gen::container<std::vector<std::size_t>>(gen::inRange(1, 10000)));

  SECTION("sum") {
    prop("returns sum of frequencies",
         [] {
           const auto frequencies = *genFrequencies;
           const auto expected = std::accumulate(
               begin(frequencies), end(frequencies), std::size_t(0));

           RC_ASSERT(FrequencyMap(frequencies).sum() == expected);
         });
  }

  SECTION("lookup") {
    prop("equal to naive implementation",
         [] {
           const auto frequencies = *genFrequencies;
           FrequencyMap map(frequencies);
           const auto x = *gen::inRange<std::size_t>(0, map.sum());

           std::size_t total = 0;
           std::size_t i = 0;
           for (i = 0; (total + frequencies[i]) <= x; i++) {
             total += frequencies[i];
           }

           RC_ASSERT(map.lookup(x) == i);
         });
  }
}
