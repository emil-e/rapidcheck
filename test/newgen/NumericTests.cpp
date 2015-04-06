#include <catch.hpp>
#include <rapidcheck-catch.h>

#include <numeric>

#include "rapidcheck/newgen/Arbitrary.h"
#include "rapidcheck/newgen/Numeric.h"
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

template<typename T,
         typename = typename  std::enable_if<
             std::is_integral<T>::value>::type>
typename std::make_unsigned<T>::type absolute(T x)
{
    return (x < 0) ? -x : x;
}

template<typename T,
         typename = typename std::enable_if<
             std::is_floating_point<T>::value>::type>
T absolute(T x) { return std::abs(x); }

template<typename T>
bool isAllOnes(T x)
{
    using UInt = typename std::make_unsigned<T>::type;
    return static_cast<UInt>(x) == std::numeric_limits<UInt>::max();
}

struct IntegralProperties
{
    template<typename T>
    static void exec()
    {
        TEMPLATED_SECTION(T, "when size >= gen::kNominalSize") {
            newtemplatedProp<T>(
                "all bits can be either 1 or 0",
                [](Random random) {
                    T ones = 0;
                    T zeroes = 0;
                    while (!isAllOnes(ones) || !isAllOnes(zeroes)) {
                        T value = newgen::arbitrary<T>()(
                            random.split()).value();
                        ones |= value;
                        zeroes |= ~value;
                    }
                });

            newtemplatedProp<T>(
                "values are uniformly distributed over entire range",
                [](Random random) {
                    using UInt = typename std::make_unsigned<T>::type;

                    std::array<uint64_t, 8> bins;
                    static constexpr UInt kBinSize =
                        std::numeric_limits<UInt>::max() / bins.size();
                    bins.fill(0);

                    static constexpr std::size_t nSamples = 10000;
                    for (std::size_t i = 0; i < nSamples; i++) {
                        UInt value = newgen::arbitrary<T>()(random.split()).value();
                        bins[value / kBinSize]++;
                    }

                    double ideal = nSamples / static_cast<double>(bins.size());
                    double error = std::accumulate(
                        begin(bins), end(bins), 0.0,
                        [=](double error, double x) {
                            double diff = 1.0 - (x / ideal);
                            return error + (diff * diff);
                        });

                    RC_ASSERT(error < 0.1);
                });
        }

        newtemplatedProp<T>(
            "monotonically increasing size yields monotonically inscreasing"
            " abs(value)",
            [](const Random &random) {
                using AbsT = decltype(absolute(std::declval<T>()));
                AbsT prev = 0;
                for (int i = 0; i <= gen::kNominalSize; i++) {
                    AbsT value = absolute(
                        newgen::arbitrary<T>()(random, i).value());
                    RC_ASSERT(value >= prev);
                    prev = value;
                }
            });

        newtemplatedProp<T>(
            "finds minimum where value must be larger/smaller than some value",
            [](const Random &random) {
                int size = *newgen::inRange<int>(0, 200);
                const auto shrinkable = newgen::arbitrary<T>()(random, size);
                T start = shrinkable.value();
                T target;
                std::pair<T, int> result;
                if (start < 0) {
                    target = *newgen::inRange<T>(start, 1);
                    result = shrinkable::findLocalMin(
                        shrinkable,
                        [=](T x) { return x <= target; });
                } else {
                    target = *newgen::inRange<T>(0, start + 1);
                    result = shrinkable::findLocalMin(
                        shrinkable,
                        [=](T x) { return x >= target; });
                }

                RC_ASSERT(result.first == target);
            });
    }
};

struct NumericProperties
{
    template<typename T>
    static void exec()
    {
        newtemplatedProp<T>(
            "zero size always yields zero",
            [](const Random &random) {
                auto shrinkable = newgen::arbitrary<T>()(random, 0);
                RC_ASSERT(shrinkable == shrinkable::just(static_cast<T>(0)));
            });
    }
};

struct SignedProperties
{
    template<typename T>
    static void exec()
    {
        newtemplatedProp<T>(
            "P(value > 0) ~ P(value < 0)",
            [](Random random) {
                static constexpr int kEnough = 5000;
                int size = *newgen::inRange<int>(50, 200);
                int n = 0;
                for (int i = 0; i < kEnough; i++) {
                    T value =
                        newgen::arbitrary<T>()(random.split(), size).value();
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

struct InRangeProperties
{
    template<typename T>
    static void exec()
    {
        newtemplatedProp<T>(
            "never generates values outside of range",
            [](const GenParams &params) {
                // TODO range generator
                const auto a = *newgen::arbitrary<T>();
                const auto b = *newgen::distinctFrom(a);
                const auto min = std::min(a, b);
                const auto max = std::max(a, b);
                const auto value = newgen::inRange<T>(
                    min, max)(params.random, params.size).value();
                RC_ASSERT(value >= min && value < max);
            });

        newtemplatedProp<T>(
            "throws if min <= max",
            [](const GenParams &params) {
                // TODO range generator
                const auto a = *newgen::arbitrary<T>();
                const auto b = *newgen::distinctFrom(a);
                const auto gen = newgen::inRange<T>(
                    std::max(a, b), std::min(a, b));
                const auto shrinkable = gen(params.random, params.size);
                try {
                    shrinkable.value();
                } catch (const GenerationFailure &e) {
                    // TODO RC_ASSERT_THROWS
                    RC_SUCCEED("Threw GenerationFailure");
                }
                RC_FAIL("Did not throw GenerationFailure");
            });

        newtemplatedProp<T>(
            "has no shrinks",
            [](const GenParams &params) {
                // TODO range generator
                const auto a = *newgen::arbitrary<T>();
                const auto b = *newgen::distinctFrom(a);
                const auto shrinkable = newgen::inRange<T>(
                    std::min(a, b), std::max(a, b))(
                        params.random, params.size);
                RC_ASSERT(!shrinkable.shrinks().next());
            });

        newtemplatedProp<T>(
            "generates all values in range",
            [](const GenParams &params) {
                const auto size = *newgen::inRange<T>(1, 20);
                const auto min = *newgen::inRange<T>(
                    std::numeric_limits<T>::min(),
                    std::numeric_limits<T>::max() - size);

                const auto gen = newgen::inRange<T>(min, min + size);
                Random r(params.random);
                std::vector<int> counts(size, 0);
                for (std::size_t i = 0; i < 2000000; i++) {
                    const auto x = gen(r.split(), params.size).value();
                    counts[x - min]++;
                    const auto done = std::find(begin(counts),
                                                end(counts), 0) == end(counts);
                    if (done)
                        RC_SUCCEED("All generated");
                }

                RC_FAIL("Gave up");
            });
    }
};

} // namespace

TEST_CASE("newgen::inRange") {
    meta::forEachType<InRangeProperties, RC_INTEGRAL_TYPES>();
}

namespace {

struct NonZeroProperties
{
    template<typename T>
    static void exec()
    {
        newtemplatedProp<T>(
            "never generates zero",
            [=](const GenParams &params) {
                const auto shrinkable = newgen::nonZero<T>()(
                    params.random, params.size);
                onAnyPath(
                    shrinkable,
                    [](const Shrinkable<T> &value,
                       const Shrinkable<T> &shrink) {
                        RC_ASSERT(value.value() != 0);
                    });
            });
    }
};

} // namespace

TEST_CASE("newgen::nonZero") {
    meta::forEachType<NonZeroProperties,
                      RC_NUMERIC_TYPES>();
}

namespace {

struct PositiveProperties
{
    template<typename T>
    static void exec()
    {
        newtemplatedProp<T>(
            "always generates positive values",
            [=](const GenParams &params) {
                const auto shrinkable = newgen::positive<T>()(
                    params.random, params.size);
                onAnyPath(
                    shrinkable,
                    [](const Shrinkable<T> &value,
                       const Shrinkable<T> &shrink) {
                        RC_ASSERT(value.value() > 0);
                    });
            });
    }
};

} // namespace

TEST_CASE("newgen::positive") {
    meta::forEachType<PositiveProperties,
                      RC_NUMERIC_TYPES>();
}

namespace {

struct NegativeProperties
{
    template<typename T>
    static void exec()
    {
        newtemplatedProp<T>(
            "always generates negative values",
            [=](const GenParams &params) {
                const auto shrinkable = newgen::negative<T>()(
                    params.random, params.size);
                onAnyPath(
                    shrinkable,
                    [](const Shrinkable<T> &value,
                       const Shrinkable<T> &shrink) {
                        RC_ASSERT(value.value() < 0);
                    });
            });
    }
};

} // namespace

TEST_CASE("newgen::negative") {
    meta::forEachType<NegativeProperties,
                      RC_SIGNED_TYPES>();
}

namespace {

struct NonNegativeProperties
{
    template<typename T>
    static void exec()
    {
        newtemplatedProp<T>(
            "always generates non-negative values",
            [=](const GenParams &params) {
                const auto shrinkable = newgen::nonNegative<T>()(
                    params.random, params.size);
                onAnyPath(
                    shrinkable,
                    [](const Shrinkable<T> &value,
                       const Shrinkable<T> &shrink) {
                        RC_ASSERT(value.value() >= 0);
                    });
            });
    }
};

} // namespace

TEST_CASE("newgen::nonNegative") {
    meta::forEachType<NonNegativeProperties,
                      RC_NUMERIC_TYPES>();
}
