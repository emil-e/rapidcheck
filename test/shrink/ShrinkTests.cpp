#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/shrink/Shrink.h"
#include "rapidcheck/seq/Operations.h"

#include "util/Util.h"
#include "util/Meta.h"
#include "util/TypeListMacros.h"

using namespace rc;

namespace {

struct NewRemoveChunksProperties
{
    template<typename T>
    static void exec()
    {
        static const auto fewValues = newgen::scale(0.3, newgen::arbitrary<T>());
        // TODO non-empty generator
        static const auto fewNonEmptyValues = newgen::suchThat(
            fewValues,
            [](const T &x) { return !x.empty(); });

        newtemplatedProp<T>(
            "first tries empty collection",
            [] {
                const auto collection = *fewNonEmptyValues;
                RC_ASSERT(shrink::newRemoveChunks(collection).next()->empty());
            });

        newtemplatedProp<T>(
            "successively increases in size for each shrink",
            [] {
                const auto seq = shrink::newRemoveChunks(*fewValues);
                T c;
                seq::forEach(std::move(seq), [&](T &&next) {
                    RC_ASSERT(next.size() >= c.size());
                    c = std::move(next);
                });
            });

        newtemplatedProp<T>(
            "shrinks to a subset of the original",
            [] {
                const auto elements = *fewValues;
                const auto seq = shrink::newRemoveChunks(elements);
                seq::forEach(std::move(seq), [&](T &&c) {
                    auto diff(setDifference<char>(c, elements));
                    RC_ASSERT(diff.size() == 0);
                });
            });

        newtemplatedProp<T>(
            "every removal of consecutive elements is a possible shrink",
            [] {
                const auto elements = *fewNonEmptyValues;
                const auto size = elements.size();
                const auto a = *newgen::inRange<int>(0, size + 1);
                const auto b = *newgen::distinctFrom(newgen::inRange<int>(0, size + 1), a);
                const auto left = std::min(a, b);
                const auto right = std::max(a, b);

                T shrink;
                shrink.reserve(size - (right - left));
                shrink.insert(end(shrink),
                              begin(elements),
                              begin(elements) + left);
                shrink.insert(end(shrink),
                              begin(elements) + right,
                              end(elements));

                RC_ASSERT(seq::contains(shrink::newRemoveChunks(elements),
                                        shrink));
            });

        newtemplatedProp<T>(
            "never yields the original value",
            [] {
                auto elements = *fewValues;
                RC_ASSERT(!seq::contains(shrink::newRemoveChunks(elements),
                                         elements));
            });
    }
};

} // namespace

TEST_CASE("shrink::newRemoveChunks") {
    meta::forEachType<NewRemoveChunksProperties,
                      std::vector<char>,
                      std::string>();
}

namespace {

struct NewEachElementProperties
{
    template<typename T>
    static void exec()
    {
        newtemplatedProp<T>(
            "every shrink for every element is tried in order",
            [] {
                const auto elements = *newgen::container<T>(
                    newgen::nonNegative<char>());
                auto seq = shrink::newEachElement(
                    elements,
                    [=] (char x) {
                        return seq::range(x - 1, -1);
                    });

                for (std::size_t i = 0; i < elements.size(); i++) {
                    auto x = elements[i];
                    while (x > 0) {
                        auto expected = elements;
                        expected[i] = --x;
                        RC_ASSERT(*seq.next() == expected);
                    }
                }

                RC_ASSERT(!seq.next());
            });
    }
};

} // namespace

TEST_CASE("shrink::newEachElement") {
    meta::forEachType<NewEachElementProperties,
                      std::vector<char>,
                      std::string>();
}

namespace {

struct ShrinkTowardsProperties
{
    template<typename T>
    static void exec()
    {
        newtemplatedProp<T>(
            "first tries target immediately",
            [] (T target) {
                T value = *newgen::distinctFrom(target);
                auto seq = shrink::towards(value, target);
                auto first = seq.next();
                RC_ASSERT(first);
                RC_ASSERT(*first == target);
            });

        newtemplatedProp<T>(
            "tries an adjacent value last",
            [] (T target) {
                T value = *newgen::distinctFrom(target);
                auto seq = shrink::towards(value, target);
                auto fin = seq::last(seq);
                RC_ASSERT(fin);
                T diff = (value > target) ? (value - *fin) : (*fin - value);
                RC_ASSERT(diff == 1);
            });

        newtemplatedProp<T>(
            "shrinking towards self yields empty shrink",
            [] (T target) {
                RC_ASSERT(!shrink::towards(target, target).next());
            });
    }
};

} // namespace

TEST_CASE("shrink::towards") {
    meta::forEachType<ShrinkTowardsProperties, RC_INTEGRAL_TYPES>();
}

namespace {

struct IntegralProperties
{
    template<typename T>
    static void exec()
    {
        newtemplatedProp<T>(
            "always tries zero first",
            [] {
                T value = *newgen::nonZero<T>();
                RC_ASSERT(*shrink::integral<T>(value).next() == 0);
            });

        TEMPLATED_SECTION(T, "zero has no shrinks") {
            REQUIRE(!shrink::integral<T>(0).next());
        }

        newtemplatedProp<T>(
            "never contains original value",
            [](T x) {
                RC_ASSERT(!seq::contains(shrink::integral<T>(x), x));
            });
    }
};

struct SignedIntegralProperties
{
    template<typename T>
    static void exec()
    {
        newtemplatedProp<T>(
            "shrinks negative values to their positive equivalent",
            [] {
                T value = *newgen::negative<T>();
                RC_ASSERT(seq::contains<T>(shrink::integral<T>(value), -value));
            });

        newtemplatedProp<T>(
            "always tries zero first",
            [] {
                T value = *newgen::nonZero<T>();
                RC_ASSERT(*shrink::integral<T>(value).next() == 0);
            });

        TEMPLATED_SECTION(T, "zero has no shrinks") {
            REQUIRE(!shrink::integral<T>(0).next());
        }
    }
};

} // namespace

TEST_CASE("shrink::integral") {
    meta::forEachType<IntegralProperties, RC_INTEGRAL_TYPES>();
    meta::forEachType<SignedIntegralProperties, RC_SIGNED_INTEGRAL_TYPES>();
}

namespace {

struct RealProperties
{
    template<typename T>
    static void exec()
    {
        newtemplatedProp<T>(
            "shrinks to nearest integer",
            [] {
                T value = *newgen::scale(0.25, newgen::nonZero<T>());
                RC_PRE(value != std::trunc(value));
                RC_ASSERT(seq::contains(shrink::real(value), std::trunc(value)));
            });

        TEMPLATED_SECTION(T, "zero has no shrinks") {
            REQUIRE(!shrink::real<T>(0.0).next());
        }

        newtemplatedProp<T>(
            "tries 0.0 first",
            [] {
                T value = *newgen::nonZero<T>();
                REQUIRE(*shrink::real<T>(value).next() == 0.0);
            });

        newtemplatedProp<T>(
            "never contains original value",
            [](T x) {
                RC_ASSERT(!seq::contains(shrink::real<T>(x), x));
            });
    }
};

} // namespace

TEST_CASE("shrink::real") {
    meta::forEachType<RealProperties, RC_REAL_TYPES>();
}

TEST_CASE("shrink::bool") {
    SECTION("shrinks 'true' to 'false'") {
        REQUIRE(shrink::boolean(true) == seq::just(false));
    }

    SECTION("does not shrink 'false'") {
        REQUIRE(!shrink::boolean(false).next());
    }
}
