#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/TypeListMacros.h"
#include "util/Meta.h"
#include "util/Util.h"

using namespace rc;

namespace {

struct RangedProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "never generates values outside of range", [] {
                T min = *gen::arbitrary<T>();
                T max = *gen::suchThat<T>([=](T x) { return x > min; });
                T x = *gen::noShrink(gen::ranged(min, max));
                RC_ASSERT((x >= min) && (x < max));
            });

        templatedProp<T>(
            "shrinks using shrink::towards with lower bound as target", [] {
                T min = *gen::arbitrary<T>();
                T max = *gen::suchThat<T>([=](T x) { return x > min; });
                auto generator = gen::ranged(min, max);
                T value = *generator;
                RC_ASSERT(yieldsEqual(generator.shrink(value),
                                      shrink::towards(value, min)));
            });
    }
};

struct SignedRangedProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "sometimes generates negative values if in range", [] {
                T min = *gen::negative<T>();
                T max = *gen::positive<T>();
                auto generator = gen::noShrink(gen::ranged(min, max));
                while (true)
                    RC_SUCCEED_IF(*generator < 0);

                return false;
            });
    }
};

} // namespace

TEST_CASE("gen::ranged") {
    meta::forEachType<RangedProperties, RC_INTEGRAL_TYPES>();
    meta::forEachType<SignedRangedProperties, RC_SIGNED_INTEGRAL_TYPES>();
}

namespace {

struct NonZeroProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>("never generates zero", [] {
            RC_ASSERT(*gen::noShrink(gen::nonZero<T>()) != 0);
        });
    }
};


struct PositiveProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>("never generates non-positive", [] {
            RC_ASSERT(*gen::noShrink(gen::positive<T>()) > 0);
        });
    }
};

struct NegativeProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>("never generates non-negative", [] {
            RC_ASSERT(*gen::noShrink(gen::negative<T>()) < 0);
        });
    }
};

struct NonNegativeProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>("never generates negative", [] {
            RC_ASSERT(*gen::noShrink(gen::nonNegative<T>()) >= 0);
        });
    }
};

} // namespace

TEST_CASE("gen::nonZero") {
    meta::forEachType<NonZeroProperties, RC_NUMERIC_TYPES>();
}

TEST_CASE("gen::positive") {
    meta::forEachType<PositiveProperties, RC_NUMERIC_TYPES>();
}

TEST_CASE("gen::negative") {
    meta::forEachType<NegativeProperties, RC_SIGNED_TYPES>();
}

TEST_CASE("gen::nonNegative") {
    meta::forEachType<NonNegativeProperties, RC_NUMERIC_TYPES>();
}
