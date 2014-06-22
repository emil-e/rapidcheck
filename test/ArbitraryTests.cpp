#include <catch.hpp>
#include <rapidcheck.h>

#include "Utils.h"
#include "Meta.h"

using namespace rc;

template<typename T, typename Predicate>
void generatesSuchThat(const Predicate &pred)
{
    testEnv([&] {
        auto generator = gen::resize(50, gen::arbitrary<T>());
        while (true) {
            T x = pick(generator);
            if (pred(x)) {
                SUCCEED("Generated value that satisfied predicate");
                return;
            }
        }
        FAIL("The impossible happened...");
    });
}

struct NumericProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "generates only zero when size is zero",
            [] {
                testEnv([] {
                    RC_ASSERT(pick(gen::resize(0, gen::arbitrary<T>())) == 0);
                });
            });
    }
};

struct SignedProperties
{
    template<typename T>
    static void exec()
    {
        TEMPLATED_SECTION(T, "generates positive values") {
            generatesSuchThat<T>([] (T x) { return x > 0; });
        }

        TEMPLATED_SECTION(T, "generates negative values") {
            generatesSuchThat<T>([] (T x) { return x < 0; });
        }

        templatedProp<T>(
            "shrinks negative values to their positive equivalent",
            [] {
                T value = pick(gen::negative<T>());
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
                T value = pick(gen::nonZero<T>());
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
    SECTION("generates both true and false") {
        testEnv([] {
            while (true) {
                if (pick<bool>())
                    break;
            }

            while (true) {
                if (!pick<bool>())
                    break;
            }
        });
    }

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
