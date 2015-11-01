#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "util/GenUtils.h"
#include "util/ShrinkableUtils.h"
#include "util/Predictable.h"
#include "util/Util.h"
#include "util/Meta.h"
#include "util/TypeListMacros.h"
#include "util/Generators.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("gen::suchThat") {
  prop("generated value always matches predicate",
       [](const GenParams &params) {
         const auto gen = gen::suchThat(gen::arbitrary<int>(),
                                        [](int x) { return (x % 2) == 0; });
         const auto shrinkable = gen(params.random, params.size);
         onAnyPath(
             shrinkable,
             [](const Shrinkable<int> &value, const Shrinkable<int> &shrink) {
               RC_ASSERT((value.value() % 2) == 0);
             });
       });

  prop(
      "if predicate returns true for every value, returned shrinkable is"
      " unchanged",
      [](const GenParams &params, const Shrinkable<int> &shrinkable) {
        const Gen<int> underlying(fn::constant(shrinkable));
        const auto gen = gen::suchThat(underlying, fn::constant(true));
        RC_ASSERT(underlying(params.random, params.size) ==
                  gen(params.random, params.size));
      });

  prop("throws GenerationFailure if value cannot be generated",
       [](const GenParams &params) {
         const auto gen = gen::suchThat(gen::just<int>(0), fn::constant(false));
         const auto shrinkable = gen(params.random, params.size);
         RC_ASSERT_THROWS_AS(shrinkable.value(), GenerationFailure);
       });

  prop("passes the passed size to the underlying generator on the first try",
       [] {
         const auto size = *gen::inRange<int>(0, 2000);
         const auto gen = gen::suchThat(genPassedParams(), fn::constant(true));
         const auto params = gen(Random(), size).value();
         RC_ASSERT(params.size == size);
       });

  prop("uses gen::arbitrary if no generator is specified",
       [](const GenParams &params) {
         const auto value = gen::suchThat<Predictable>(fn::constant(true))(
                                params.random, params.size)
                                .value();
         RC_ASSERT(isArbitraryPredictable(value));
       });

  SECTION("works with non-copyable types") {
    const auto value = gen::suchThat(gen::arbitrary<NonCopyable>(),
                                     fn::constant(true))(Random(), 0)
                           .value();
    REQUIRE(isArbitraryPredictable(value));
  }
}

TEST_CASE("gen::nonEmpty") {
  prop("never generates empty values",
       [](const GenParams &params) {
         const auto gen = gen::nonEmpty(gen::string<std::string>());
         onAnyPath(gen(params.random, params.size),
                   [](const Shrinkable<std::string> &value,
                      const Shrinkable<std::string> &shrink) {
                     RC_ASSERT(!value.value().empty());
                   });
       });

  prop("uses correct arbitrary instance",
       [](const GenParams &params) {
         const auto gen = gen::nonEmpty<std::vector<Predictable>>();
         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(std::all_of(
             begin(value),
             end(value),
             [](const Predictable &p) { return isArbitraryPredictable(p); }));
       });
}

TEST_CASE("gen::distinctFrom") {
  prop("value is never equal to the given one",
       [](const GenParams &params, int x) {
         const auto gen = gen::distinctFrom(x);
         onAnyPath(
             gen(params.random, params.size),
             [=](const Shrinkable<int> &value, const Shrinkable<int> &shrink) {
               RC_ASSERT(value.value() != x);
             });
       });

  prop("uses the correct generator when specified",
       [](const GenParams &params, int x) {
         const auto gen = gen::distinctFrom(gen::just(x), x - 1);
         RC_ASSERT(gen(params.random, params.size).value() == x);
       });

  prop("uses the correct arbitrary instance if generator not specified",
       [](const GenParams &params, const Predictable &x) {
         const auto gen = gen::distinctFrom(x);
         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(isArbitraryPredictable(value));
       });
}

namespace {

struct NonZeroProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>("never generates zero",
                     [=](const GenParams &params) {
                       const auto shrinkable =
                           gen::nonZero<T>()(params.random, params.size);
                       onAnyPath(shrinkable,
                                 [](const Shrinkable<T> &value,
                                    const Shrinkable<T> &shrink) {
                                   RC_ASSERT(value.value() != 0);
                                 });
                     });
  }
};

} // namespace

TEST_CASE("gen::nonZero") {
  forEachType<NonZeroProperties, RC_NUMERIC_TYPES>();
}

namespace {

struct PositiveProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>("always generates positive values",
                     [=](const GenParams &params) {
                       const auto shrinkable =
                           gen::positive<T>()(params.random, params.size);
                       onAnyPath(shrinkable,
                                 [](const Shrinkable<T> &value,
                                    const Shrinkable<T> &shrink) {
                                   RC_ASSERT(value.value() > T(0));
                                 });
                     });
  }
};

} // namespace

TEST_CASE("gen::positive") {
  forEachType<PositiveProperties, RC_NUMERIC_TYPES>();
}

namespace {

struct NegativeProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>("always generates negative values",
                     [=](const GenParams &params) {
                       const auto shrinkable =
                           gen::negative<T>()(params.random, params.size);
                       onAnyPath(shrinkable,
                                 [](const Shrinkable<T> &value,
                                    const Shrinkable<T> &shrink) {
                                   RC_ASSERT(value.value() < 0);
                                 });
                     });
  }
};

} // namespace

TEST_CASE("gen::negative") {
  forEachType<NegativeProperties, RC_SIGNED_TYPES>();
}

namespace {

struct NonNegativeProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>("always generates non-negative values",
                     [=](const GenParams &params) {
                       const auto shrinkable =
                           gen::nonNegative<T>()(params.random, params.size);
                       onAnyPath(shrinkable,
                                 [](const Shrinkable<T> &value,
                                    const Shrinkable<T> &shrink) {
                                   RC_ASSERT(value.value() >= T(0));
                                 });
                     });
  }
};

} // namespace

TEST_CASE("gen::nonNegative") {
  forEachType<NonNegativeProperties, RC_NUMERIC_TYPES>();
}
