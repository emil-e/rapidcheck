#include <catch.hpp>
#include <rapidcheck/catch.h>

#include <numeric>

#include "rapidcheck/Random.h"

#include "util/ArbitraryRandom.h"
#include "util/Util.h"
#include "util/Meta.h"

using namespace rc;
using namespace rc::test;

namespace {

struct AssociativeProperties {
  template <typename Map>
  static void exec() {
    templatedProp<Map>("can be used as key in STL associative containers",
                       [](const std::vector<std::pair<Random, int>> &pairs) {
                         Map map(begin(pairs), end(pairs));
                         std::vector<std::pair<Random, int>> extracted;
                         extracted.reserve(pairs.size());
                         std::transform(begin(pairs),
                                        end(pairs),
                                        std::back_inserter(extracted),
                                        [&](const std::pair<Random, int> &p) {
                                          return std::make_pair(p.first,
                                                                map[p.first]);
                                        });

                         RC_ASSERT(extracted == pairs);
                       });
  };
};
}

TEST_CASE("Random") {
  SECTION("default constructor is same as all-zero seed") {
    REQUIRE(Random() == Random({0, 0, 0, 0}));
  }

  prop("different keys yield inequal generators",
       [] {
         auto key1 = *gen::arbitrary<Random::Key>();
         auto key2 = *gen::distinctFrom(key1);
         RC_ASSERT(Random(key1) != Random(key2));
       });

  prop("different seeds yield inequal generators",
       [] {
         auto seed1 = *gen::arbitrary<uint64_t>();
         auto seed2 = *gen::distinctFrom(seed1);
         RC_ASSERT(Random(seed1) != Random(seed2));
       });

  prop("different splits are inequal",
       [](Random r1) {
         Random r2(r1.split());
         RC_ASSERT(r1 != r2);
       });

  prop("next modifies state",
       [] {
         Random r1 = *trulyArbitraryRandom();
         Random r2(r1);
         r2.next();
         RC_ASSERT(r1 != r2);
       });

  prop("different keys yield different sequences",
       [] {
         auto key1 = *gen::arbitrary<Random::Key>();
         auto key2 = *gen::distinctFrom(key1);
         Random r1(key1);
         Random r2(key2);
         for (std::size_t i = 0; i < 4; i++) {
           RC_SUCCEED_IF(r1.next() != r2.next());
         }
         RC_FAIL("Equal random numbers");
       });

  prop("different seeds yield different sequences",
       [] {
         auto seed1 = *gen::arbitrary<uint64_t>();
         auto seed2 = *gen::distinctFrom(seed1);
         Random r1(seed1);
         Random r2(seed2);
         for (std::size_t i = 0; i < 4; i++) {
           RC_SUCCEED_IF(r1.next() != r2.next());
         }
         RC_FAIL("Equal random numbers");
       });

  prop("different splits yield different sequences",
       [](Random r1) {
         Random r2(r1.split());

         // Random numbers are generated in 256-bit blocks so if the first
         // four are equal, it's likely the same block and something is
         // wrong even if subsequent blocks are not equal
         for (std::size_t i = 0; i < 4; i++) {
           RC_SUCCEED_IF(r1.next() != r2.next());
         }
         RC_FAIL("Equal random numbers");
       });

  prop("different number of nexts yield different sequences",
       [] {
         Random r1 = *trulyArbitraryRandom();
         Random r2(r1);
         auto gen = gen::inRange<int>(0, 2000);
         int n1 = *gen;
         int n2 = *gen::distinctFrom(gen, n1);

         while (n1-- > 0) {
           r1.next();
         }
         while (n2-- > 0) {
           r2.next();
         }

         for (std::size_t i = 0; i < 4; i++) {
           RC_SUCCEED_IF(r1.next() != r2.next());
         }
         RC_FAIL("Equal random numbers");
       });

  prop("copies are equal",
       [] {
         Random r1 = *trulyArbitraryRandom();
         Random r2(r1);
         RC_ASSERT(r1 == r2);
       });

  prop("copies yield equal random numbers",
       [] {
         Random r1 = *trulyArbitraryRandom();
         Random r2(r1);

         // The first N numbers could theoretically be equal even if
         // subsequent numbers are not but the likelihood of different
         // states generating identical 64 * N bits is very low.
         for (std::size_t i = 0; i < 1000; i++) {
           auto x1 = r1.next();
           RC_ASSERT(x1 == r2.next());
         }
       });

  // Somewhat dubious test, I know. Hopefully, if something totally breaks,
  // it'll let us know at least.
  prop("has uniform distribution",
       [] {
         Random random = *trulyArbitraryRandom();
         std::array<uint64_t, 16> bins;
         static constexpr uint64_t kBinSize =
             (std::numeric_limits<uint64_t>::max() / 16) + 1;
         bins.fill(0);
         static constexpr std::size_t nSamples = 200000;
         for (std::size_t i = 0; i < nSamples; i++) {
           bins[random.next() / kBinSize]++;
         }

         double ideal = nSamples / static_cast<double>(bins.size());
         double error = std::accumulate(begin(bins),
                                        end(bins),
                                        0.0,
                                        [=](double error, uint64_t x) {
                                          double diff = 1.0 - (x / ideal);
                                          return error + (diff * diff);
                                        });

         RC_ASSERT(error < 0.01);
       });

  meta::forEachType<AssociativeProperties,
                    std::map<Random, int>,
                    std::unordered_map<Random, int>>();
}
