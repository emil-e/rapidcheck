#include <catch.hpp>
#include <rapidcheck-catch.h>

#include <array>

#include "util/Util.h"
#include "util/Meta.h"
#include "util/Predictable.h"

using namespace rc;

template<typename T, typename Predicate>
void generatesSuchThat(const Predicate &pred)
{
    auto generator = gen::noShrink(gen::resize(50, gen::arbitrary<T>()));
    while (true) {
        T x = *generator;
        if (pred(x)) {
            RC_SUCCEED("Generated value that satisfied predicate");
            return;
        }
    }
    RC_FAIL("The impossible happened...");
}

struct NumericProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "generates only zero when size is zero",
            [] {
                auto value =
                    *gen::noShrink(gen::resize(0, gen::arbitrary<T>()));
                RC_ASSERT(value == 0);
            });
    }
};

struct SignedProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "generates positive values",
            [] {
                generatesSuchThat<T>([] (T x) { return x > 0; });
            });

        templatedProp<T>(
            "generates negative values",
            [] {
                generatesSuchThat<T>([] (T x) { return x < 0; });
            });

        templatedProp<T>(
            "shrinks negative values to their positive equivalent",
            [] {
                T value = *gen::negative<T>();
                auto it = gen::arbitrary<T>().shrink(value);
                RC_ASSERT(it->hasNext());
                RC_ASSERT(it->next() == -value);
            });
    }
};

TEST_CASE("gen::arbitrary<T> (signed integral)") {
    meta::forEachType<NumericProperties, RC_SIGNED_INTEGRAL_TYPES>();
    meta::forEachType<SignedProperties, RC_SIGNED_INTEGRAL_TYPES>();
}

TEST_CASE("gen::arbitrary<T> (unsigned integral)") {
    meta::forEachType<NumericProperties, RC_UNSIGNED_INTEGRAL_TYPES>();
}

struct RealProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "shrinks to nearest integer",
            [] {
                T value = *gen::nonZero<T>();
                RC_PRE(value != std::trunc(value));
                auto it = gen::arbitrary<T>().shrink(value);
                while (it->hasNext())
                    RC_SUCCEED_IF(it->next() == std::trunc(value));
                RC_FAIL("none of the shrinks are the expected one");
            });
    }
};

TEST_CASE("gen::arbitrary<T> (reals)") {
    meta::forEachType<NumericProperties, RC_REAL_TYPES>();
    meta::forEachType<SignedProperties, RC_REAL_TYPES>();
    meta::forEachType<RealProperties, RC_REAL_TYPES>();
}

TEST_CASE("gen::arbitrary<bool>") {
    prop("generates both true and false",
         [] {
             while (true) {
                 if (*gen::noShrink(gen::arbitrary<bool>()))
                     break;
             }

             while (true) {
                 if (!*gen::arbitrary<bool>())
                     break;
             }
         });

    SECTION("shrinks 'true' to 'false'") {
        auto it = gen::arbitrary<bool>().shrink(true);
        REQUIRE(it->hasNext());
        REQUIRE(it->next() == false);
        REQUIRE(!it->hasNext());
    }

    SECTION("does not shrink 'false'") {
        auto it = gen::arbitrary<bool>().shrink(false);
        REQUIRE(!it->hasNext());
    }
}

struct CollectionTests
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "uses the correct arbitrary instance",
            [] {
                auto values = *gen::arbitrary<T>();
                for (const auto &value : values)
                    RC_ASSERT(isArbitraryPredictable(value));
            });
    }
};

TEST_CASE("gen::arbitrary for containers") {
    meta::forEachType<CollectionTests,
                      RC_GENERIC_CONTAINERS(Predictable),
                      RC_GENERIC_CONTAINERS(NonCopyable),
                      std::array<Predictable, 100>,
                      std::array<NonCopyable, 100>>();
}
