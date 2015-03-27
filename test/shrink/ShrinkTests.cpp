#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/shrink/Shrink.h"
#include "rapidcheck/seq/Operations.h"

#include "util/Util.h"
#include "util/Meta.h"
#include "util/TypeListMacros.h"

using namespace rc;

namespace {

struct RemoveChunksProperties
{
    template<typename T>
    static void exec()
    {
        typedef DeepDecay<typename T::value_type> Element;
        static auto fewValues = gen::scale(0.3, gen::arbitrary<T>());
        // TODO non-empty generator
        static auto fewNonEmptyValues = gen::suchThat(
                    fewValues,
                    [](const T &x) { return !x.empty(); });

        templatedProp<T>(
            "first tries empty collection",
            [] {
                auto collection = *fewNonEmptyValues;
                RC_ASSERT(shrink::removeChunks(collection).next()->empty());
            });

        templatedProp<T>(
            "successively increases in size for each shrink",
            [] {
                auto seq = shrink::removeChunks(*fewValues);
                T c;
                seq::forEach(std::move(seq), [&](T &&next) {
                    RC_ASSERT(containerSize(next) >= containerSize(c));
                    c = std::move(next);
                });
            });

        templatedProp<T>(
            "shrinks to a subset of the original",
            [] {
                auto elements = *fewValues;
                auto seq = shrink::removeChunks(elements);
                seq::forEach(std::move(seq), [&](T &&c) {
                    auto diff(setDifference<Element>(c, elements));
                    RC_ASSERT(diff.size() == 0);
                });
            });

        templatedProp<T>(
            "every removal of consecutive elements is a possible shrink",
            [] {
                auto elements = *fewNonEmptyValues;
                auto size = containerSize(elements);
                int a = *gen::ranged<int>(0, size + 1);
                int b = *gen::distinctFrom(gen::ranged<int>(0, size + 1), a);
                int begin = std::min(a, b);
                int end = std::max(a, b);

                detail::CollectionBuilder<T> builder;
                int i = 0;
                for (const auto &x : elements) {
                    if ((i < begin) && (i >= end))
                        builder.add(x);
                    i++;
                }

                RC_ASSERT(seq::contains(shrink::removeChunks(elements),
                                        builder.result()));
            });

        templatedProp<T>(
            "never yields the original value",
            [] {
                auto elements = *fewValues;
                RC_ASSERT(!seq::contains(shrink::removeChunks(elements),
                                         elements));
            });
    }
};

struct NewRemoveChunksProperties
{
    template<typename T>
    static void exec()
    {
        static const auto fewValues = gen::scale(0.3, gen::arbitrary<T>());
        // TODO non-empty generator
        static const auto fewNonEmptyValues = gen::suchThat(
            fewValues,
            [](const T &x) { return !x.empty(); });

        templatedProp<T>(
            "first tries empty collection",
            [] {
                const auto collection = *fewNonEmptyValues;
                RC_ASSERT(shrink::newRemoveChunks(collection).next()->empty());
            });

        templatedProp<T>(
            "successively increases in size for each shrink",
            [] {
                const auto seq = shrink::newRemoveChunks(*fewValues);
                T c;
                seq::forEach(std::move(seq), [&](T &&next) {
                    RC_ASSERT(next.size() >= c.size());
                    c = std::move(next);
                });
            });

        templatedProp<T>(
            "shrinks to a subset of the original",
            [] {
                const auto elements = *fewValues;
                const auto seq = shrink::newRemoveChunks(elements);
                seq::forEach(std::move(seq), [&](T &&c) {
                    auto diff(setDifference<char>(c, elements));
                    RC_ASSERT(diff.size() == 0);
                });
            });

        templatedProp<T>(
            "every removal of consecutive elements is a possible shrink",
            [] {
                const auto elements = *fewNonEmptyValues;
                const auto size = elements.size();
                const auto a = *gen::ranged<int>(0, size + 1);
                const auto b = *gen::distinctFrom(gen::ranged<int>(0, size + 1), a);
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

        templatedProp<T>(
            "never yields the original value",
            [] {
                auto elements = *fewValues;
                RC_ASSERT(!seq::contains(shrink::newRemoveChunks(elements),
                                         elements));
            });
    }
};

} // namespace

TEST_CASE("shrink::removeChunks") {
    meta::forEachType<RemoveChunksProperties,
                      RC_GENERIC_CONTAINERS(int),
                      std::string,
                      std::wstring>();
}

TEST_CASE("shrink::newRemoveChunks") {
    meta::forEachType<NewRemoveChunksProperties,
                      std::vector<char>,
                      std::string>();
}

namespace {

struct EachElementProperties
{
    template<typename T>
    static void exec()
    {
        typedef DeepDecay<typename T::value_type> Element;
        static auto smallValue = gen::scale(0.3, gen::arbitrary<Element>());
        static auto smallValues = gen::collection<T>(smallValue);

        templatedProp<T>(
            "the container size stays the same",
            [&] {
                auto elements = *smallValues;
                auto seq = shrink::eachElement(
                    elements,
                    [&] (const Element &x) { return smallValue.shrink(x); });

                auto size = containerSize(elements);
                seq::forEach(seq, [=](const T &shrink) {
                    if (containerSize(shrink) != size)
                    RC_ASSERT(containerSize(shrink) == size);
                });
            });

        TEMPLATED_SECTION(T, "has no shrinks for empty collections") {
            auto seq = shrink::eachElement(
                T(),
                [] (const Element &x) {
                    return gen::arbitrary<Element>().shrink(x);
                });
            REQUIRE(!seq.next());
        }

        templatedProp<T>(
            "has no shrinks if elements have no shrinks",
            [] {
                auto seq = shrink::eachElement(
                    *smallValues,
                    [] (const Element &x) {
                        return Seq<Element>();
                    });
                RC_ASSERT(!seq.next());
            });

        templatedProp<T>(
            "the number of shrinks is never greater than the sum of the "
            "shrink counts for the iterators of the elements",
            [] {
                auto elements = *smallValues;
                auto seq = shrink::eachElement(
                    elements,
                    [=] (const Element &x) {
                        return smallValue.shrink(x);
                    });

                std::size_t count = 0;
                for (const auto &element : elements) {
                    count += seq::length(smallValue.shrink(element));
                }
                RC_ASSERT(seq::length(seq) <= count);
            });

        templatedProp<T>(
            "for every shrink, a value is replaced with one of its possible "
            "shrinks",
            [] {
                auto elements = *smallValues;
                auto seq = shrink::eachElement(
                    elements,
                    [&] (const Element &x) {
                        return smallValue.shrink(x);
                    });

                seq::forEach(seq, [&](const T &shrunk) {
                    auto added = setDifference<Element>(shrunk, elements);
                    auto removed = setDifference<Element>(elements, shrunk);
                    RC_ASSERT(added.size() == 1);
                    RC_ASSERT(removed.size() == 1);
                    RC_ASSERT(seq::contains(
                                  smallValue.shrink(removed[0]),
                                  added[0]));
                });
            });
    }
};

struct NewEachElementProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "every shrink for every element is tried in order",
            [] {
                const auto elements = *gen::collection<T>(
                    gen::nonNegative<char>());
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

TEST_CASE("shrink::eachElement") {
    meta::forEachType<EachElementProperties,
                      RC_GENERIC_CONTAINERS(int),
                      RC_STRING_TYPES,
                      std::array<int, 100>>();
}

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
        templatedProp<T>(
            "first tries target immediately",
            [] (T target) {
                T value = *gen::distinctFrom(target);
                auto seq = shrink::towards(value, target);
                auto first = seq.next();
                RC_ASSERT(first);
                RC_ASSERT(*first == target);
            });

        templatedProp<T>(
            "tries an adjacent value last",
            [] (T target) {
                T value = *gen::distinctFrom(target);
                auto seq = shrink::towards(value, target);
                auto fin = seq::last(seq);
                RC_ASSERT(fin);
                T diff = (value > target) ? (value - *fin) : (*fin - value);
                RC_ASSERT(diff == 1);
            });

        templatedProp<T>(
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
        templatedProp<T>(
            "always tries zero first",
            [] {
                T value = *gen::nonZero<T>();
                RC_ASSERT(*shrink::integral<T>(value).next() == 0);
            });

        TEMPLATED_SECTION(T, "zero has no shrinks") {
            REQUIRE(!shrink::integral<T>(0).next());
        }

        templatedProp<T>(
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
        templatedProp<T>(
            "shrinks negative values to their positive equivalent",
            [] {
                T value = *gen::negative<T>();
                RC_ASSERT(seq::contains<T>(shrink::integral<T>(value), -value));
            });

        templatedProp<T>(
            "always tries zero first",
            [] {
                T value = *gen::nonZero<T>();
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
        templatedProp<T>(
            "shrinks to nearest integer",
            [] {
                T value = *gen::nonZero<T>();
                RC_PRE(value != std::trunc(value));
                RC_ASSERT(seq::contains(shrink::real(value), std::trunc(value)));
            });

        TEMPLATED_SECTION(T, "zero has no shrinks") {
            REQUIRE(!shrink::real<T>(0.0).next());
        }

        templatedProp<T>(
            "tries 0.0 first",
            [] {
                T value = *gen::nonZero<T>();
                REQUIRE(*shrink::real<T>(value).next() == 0.0);
            });

        templatedProp<T>(
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
