#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/gen/Chrono.h"

#include "util/Util.h"
#include "util/Meta.h"
#include "util/GenUtils.h"
#include "util/ShrinkableUtils.h"

using namespace rc;
using namespace rc::test;

namespace {

struct DurationProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>("equivalent to generator of underlying type",
                     [](const GenParams &params) {
                       const auto gen = gen::map(gen::arbitrary<T>(),
                                                 [](T x) { return x.count(); });
                       const auto repGen = gen::arbitrary<typename T::rep>();
                       assertEquivalent(gen(params.random, params.size),
                                        repGen(params.random, params.size));
                     });
  }
};

} // namespace

TEST_CASE("arbitrary duration") {
  prop("equivalent",
       [] {
         forEachType<DurationProperties,
                     std::chrono::nanoseconds,
                     std::chrono::microseconds,
                     std::chrono::milliseconds,
                     std::chrono::seconds,
                     std::chrono::minutes,
                     std::chrono::hours>();
       });
}

TEST_CASE("arbitrary time_point") {
  prop("equivalent",
       [](const GenParams &params) {
         using TimePoint = std::chrono::system_clock::time_point;
         const auto gen =
             gen::map(gen::arbitrary<TimePoint>(),
                      [](TimePoint x) { return x.time_since_epoch().count(); });
         const auto repGen =
             gen::arbitrary<typename TimePoint::duration::rep>();
         assertEquivalent(gen(params.random, params.size),
                          repGen(params.random, params.size));
       });
}
