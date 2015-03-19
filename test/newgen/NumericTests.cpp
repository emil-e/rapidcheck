#include <catch.hpp>
#include <rapidcheck-catch.h>

#include <numeric>

#include "rapidcheck/newgen/Arbitrary.h"
#include "rapidcheck/newgen/Numeric.h"

#include "util/Util.h"
#include "util/Meta.h"
#include "util/TypeListMacros.h"
#include "util/ArbitraryRandom.h"

using namespace rc;

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
            templatedProp<T>(
                "all bits can be either 1 or 0",
                [](Random random) {
                    T ones = 0;
                    T zeroes = 0;
                    while (!isAllOnes(ones) || !isAllOnes(zeroes)) {
                        T value = newgen::arbitrary<T>()(
                            random.split(), gen::kNominalSize).value();
                        ones |= value;
                        zeroes |= ~value;
                    }
                });

            templatedProp<T>(
                "values are uniformly distributed over entire range",
                [](Random random) {
                    using UInt = typename std::make_unsigned<T>::type;

                    std::array<uint64_t, 8> bins;
                    static constexpr UInt kBinSize =
                        std::numeric_limits<UInt>::max() / bins.size();
                    bins.fill(0);

                    static constexpr std::size_t nSamples = 10000;
                    for (std::size_t i = 0; i < nSamples; i++) {
                        UInt value = newgen::arbitrary<T>()(random.split(), gen::kNominalSize).value();
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

        templatedProp<T>(
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
    }
};

struct NumericProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
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
        templatedProp<T>(
            "P(value > 0) ~ P(value < 0)",
            [](Random random) {
                static constexpr int kEnough = 5000;
                int size = *gen::ranged<int>(50, 200);
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
