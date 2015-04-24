#include <catch.hpp>
#include <rapidcheck-catch.h>

#include <numeric>

#include "rapidcheck/gen/Arbitrary.h"
#include "rapidcheck/gen/Numeric.h"
#include "rapidcheck/shrinkable/Operations.h"

#include "util/Util.h"
#include "util/Meta.h"
#include "util/TypeListMacros.h"
#include "util/ArbitraryRandom.h"
#include "util/GenUtils.h"
#include "util/ShrinkableUtils.h"

using namespace rc;
using namespace rc::test;

namespace {

template <typename T,
    typename = typename std::enable_if<std::is_integral<T>::value>::type>
typename std::make_unsigned<T>::type absolute(T x) {
  return (x < 0) ? -x : x;
}

template <typename T,
    typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
T absolute(T x) {
  return std::abs(x);
}

template <typename T>
bool isAllOnes(T x) {
  using UInt = typename std::make_unsigned<T>::type;
  return static_cast<UInt>(x) == std::numeric_limits<UInt>::max();
}

struct IntegralProperties {
  template <typename T>
  static void exec() {
    TEMPLATED_SECTION(T, "when size >= gen::kNominalSize") {
      templatedProp<T>("all bits can be either 1 or 0",
          [](Random random) {
            T ones = 0;
            T zeroes = 0;
            while (!isAllOnes(ones) || !isAllOnes(zeroes)) {
              T value = gen::arbitrary<T>()(random.split()).value();
              ones |= value;
              zeroes |= ~value;
            }
          });

      templatedProp<T>("values are uniformly distributed over entire range",
          [](Random random) {
            using UInt = typename std::make_unsigned<T>::type;

            std::array<uint64_t, 8> bins;
            static constexpr UInt kBinSize =
                std::numeric_limits<UInt>::max() / bins.size();
            bins.fill(0);

            static constexpr std::size_t nSamples = 10000;
            for (std::size_t i = 0; i < nSamples; i++) {
              UInt value = gen::arbitrary<T>()(random.split()).value();
              bins[value / kBinSize]++;
            }

            double ideal = nSamples / static_cast<double>(bins.size());
            double error = std::accumulate(begin(bins),
                end(bins),
                0.0,
                [=](double error, double x) {
                  double diff = 1.0 - (x / ideal);
                  return error + (diff * diff);
                });

            RC_ASSERT(error < 0.1);
          });
    }

    templatedProp<T>(
        "monotonically increasing size yields monotonically inscreasing"
        " abs(value)",
        [](const Random &random) {
          using AbsT = decltype(absolute(std::declval<T>()));
          AbsT prev = 0;
          for (int i = 0; i <= kNominalSize; i++) {
            AbsT value = absolute(gen::arbitrary<T>()(random, i).value());
            RC_ASSERT(value >= prev);
            prev = value;
          }
        });

    templatedProp<T>(
        "finds minimum where value must be larger/smaller than some value",
        [](const Random &random) {
          int size = *gen::inRange<int>(0, 200);
          const auto shrinkable = gen::arbitrary<T>()(random, size);
          T start = shrinkable.value();
          T target;
          std::pair<T, int> result;
          if (start < 0) {
            target = *gen::inRange<T>(start, 1);
            result = shrinkable::findLocalMin(
                shrinkable, [=](T x) { return x <= target; });
          } else {
            target = *gen::inRange<T>(0, start + 1);
            result = shrinkable::findLocalMin(
                shrinkable, [=](T x) { return x >= target; });
          }

          RC_ASSERT(result.first == target);
        });
  }
};

struct NumericProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>("zero size always yields zero",
        [](const Random &random) {
          auto shrinkable = gen::arbitrary<T>()(random, 0);
          RC_ASSERT(shrinkable == shrinkable::just(static_cast<T>(0)));
        });
  }
};

struct SignedProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>("P(value > 0) ~ P(value < 0)",
        [](Random random) {
          static constexpr int kEnough = 5000;
          int size = *gen::inRange<int>(50, 200);
          int n = 0;
          for (int i = 0; i < kEnough; i++) {
            T value = gen::arbitrary<T>()(random.split(), size).value();
            if (value < 0)
              n--;
            else if (value > 0)
              n++;
          }

          double avg = static_cast<double>(n) / kEnough;
          RC_ASSERT(avg < 0.08);
        });
  }
};

} // namespace

TEST_CASE("arbitrary integers") {
  meta::forEachType<IntegralProperties, RC_INTEGRAL_TYPES>();
  meta::forEachType<NumericProperties, RC_INTEGRAL_TYPES>();
  meta::forEachType<SignedProperties, RC_SIGNED_INTEGRAL_TYPES>();
}

TEST_CASE("arbitrary reals") {
  meta::forEachType<NumericProperties, RC_REAL_TYPES>();
  meta::forEachType<SignedProperties, RC_REAL_TYPES>();
}

namespace {

struct InRangeProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>("never generates values outside of range",
        [](const GenParams &params) {
          // TODO range generator
          const auto a = *gen::arbitrary<T>();
          const auto b = *gen::distinctFrom(a);
          const auto min = std::min(a, b);
          const auto max = std::max(a, b);
          const auto value =
              gen::inRange<T>(min, max)(params.random, params.size).value();
          RC_ASSERT(value >= min && value < max);
        });

    templatedProp<T>("throws if min <= max",
        [](const GenParams &params) {
          // TODO range generator
          const auto a = *gen::arbitrary<T>();
          const auto b = *gen::distinctFrom(a);
          const auto gen = gen::inRange<T>(std::max(a, b), std::min(a, b));
          const auto shrinkable = gen(params.random, params.size);
          try {
            shrinkable.value();
          } catch (const GenerationFailure &e) {
            // TODO RC_ASSERT_THROWS
            RC_SUCCEED("Threw GenerationFailure");
          }
          RC_FAIL("Did not throw GenerationFailure");
        });

    templatedProp<T>("has no shrinks",
        [](const GenParams &params) {
          // TODO range generator
          const auto a = *gen::arbitrary<T>();
          const auto b = *gen::distinctFrom(a);
          const auto shrinkable = gen::inRange<T>(
              std::min(a, b), std::max(a, b))(params.random, params.size);
          RC_ASSERT(!shrinkable.shrinks().next());
        });

    templatedProp<T>("generates all values in range",
        [](const GenParams &params) {
          const auto size = *gen::inRange<T>(1, 20);
          const auto min = *gen::inRange<T>(std::numeric_limits<T>::min(),
                               std::numeric_limits<T>::max() - size);

          const auto gen = gen::inRange<T>(min, min + size);
          Random r(params.random);
          std::vector<int> counts(size, 0);
          for (std::size_t i = 0; i < 2000000; i++) {
            const auto x = gen(r.split(), params.size).value();
            counts[x - min]++;
            const auto done =
                std::find(begin(counts), end(counts), 0) == end(counts);
            if (done)
              RC_SUCCEED("All generated");
          }

          RC_FAIL("Gave up");
        });
  }
};

} // namespace

TEST_CASE("gen::inRange") {
  meta::forEachType<InRangeProperties, RC_INTEGRAL_TYPES>();
}

namespace {

struct NonZeroProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>("never generates zero",
        [=](const GenParams &params) {
          const auto shrinkable = gen::nonZero<T>()(params.random, params.size);
          onAnyPath(shrinkable,
              [](const Shrinkable<T> &value, const Shrinkable<T> &shrink) {
                RC_ASSERT(value.value() != 0);
              });
        });
  }
};

} // namespace

TEST_CASE("gen::nonZero") {
  meta::forEachType<NonZeroProperties, RC_NUMERIC_TYPES>();
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
              [](const Shrinkable<T> &value, const Shrinkable<T> &shrink) {
                RC_ASSERT(value.value() > 0);
              });
        });
  }
};

} // namespace

TEST_CASE("gen::positive") {
  meta::forEachType<PositiveProperties, RC_NUMERIC_TYPES>();
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
              [](const Shrinkable<T> &value, const Shrinkable<T> &shrink) {
                RC_ASSERT(value.value() < 0);
              });
        });
  }
};

} // namespace

TEST_CASE("gen::negative") {
  meta::forEachType<NegativeProperties, RC_SIGNED_TYPES>();
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
              [](const Shrinkable<T> &value, const Shrinkable<T> &shrink) {
                RC_ASSERT(value.value() >= 0);
              });
        });
  }
};

} // namespace

TEST_CASE("gen::nonNegative") {
  meta::forEachType<NonNegativeProperties, RC_NUMERIC_TYPES>();
}
