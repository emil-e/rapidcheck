#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/shrink/NewShrink.h"
#include "rapidcheck/seq/Operations.h"

#include "util/Util.h"
#include "util/Meta.h"
#include "util/TypeListMacros.h"

using namespace rc;

namespace {

template<typename T>
Seq<T> fromIterator(shrink::IteratorUP<T> &&it)
{
    std::vector<T> elements;
    while (it->hasNext())
        elements.push_back(it->next());
    return seq::fromContainer(std::move(elements));
}

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
                RC_ASSERT(newshrink::removeChunks(collection).next()->empty());
            });

        templatedProp<T>(
            "successively increases in size for each shrink",
            [] {
                auto seq = newshrink::removeChunks(*fewValues);
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
                auto seq = newshrink::removeChunks(elements);
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

                RC_ASSERT(seq::contains(newshrink::removeChunks(elements),
                                        builder.result()));
            });

        templatedProp<T>(
            "never yields the original value",
            [] {
                auto elements = *fewValues;
                RC_ASSERT(!seq::contains(newshrink::removeChunks(elements),
                                         elements));
            });
    }
};

} // namespace

TEST_CASE("newshrink::removeChunks") {
    meta::forEachType<RemoveChunksProperties,
                      RC_GENERIC_CONTAINERS(int),
                      std::string,
                      std::wstring>();
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
                auto seq = newshrink::eachElement(
                    elements,
                    [&] (const Element &x) { return fromIterator(smallValue.shrink(x)); });

                auto size = containerSize(elements);
                seq::forEach(seq, [=](const T &shrink) {
                    if (containerSize(shrink) != size)
                    RC_ASSERT(containerSize(shrink) == size);
                });
            });

        TEMPLATED_SECTION(T, "has no shrinks for empty collections") {
            auto seq = newshrink::eachElement(
                T(),
                [] (const Element &x) {
                    return fromIterator(gen::arbitrary<Element>().shrink(x));
                });
            REQUIRE(!seq.next());
        }

        templatedProp<T>(
            "has no shrinks if elements have no shrinks",
            [] {
                auto seq = newshrink::eachElement(
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
                auto seq = newshrink::eachElement(
                    elements,
                    [=] (const Element &x) {
                        return fromIterator(smallValue.shrink(x));
                    });

                std::size_t count = 0;
                for (const auto &element : elements) {
                    count += seq::length(
                        fromIterator(smallValue.shrink(element)));
                }
                RC_ASSERT(seq::length(seq) <= count);
            });

        templatedProp<T>(
            "for every shrink, a value is replaced with one of its possible "
            "shrinks",
            [] {
                auto elements = *smallValues;
                auto seq = newshrink::eachElement(
                    elements,
                    [&] (const Element &x) {
                        return fromIterator(smallValue.shrink(x));
                    });

                seq::forEach(seq, [&](const T &shrunk) {
                    auto added = setDifference<Element>(shrunk, elements);
                    auto removed = setDifference<Element>(elements, shrunk);
                    RC_ASSERT(added.size() == 1);
                    RC_ASSERT(removed.size() == 1);
                    RC_ASSERT(seq::contains(
                                  fromIterator(smallValue.shrink(removed[0])),
                                  added[0]));
                });
            });
    }
};

} // namespace

TEST_CASE("newshrink::eachElement") {
    meta::forEachType<EachElementProperties,
                      RC_GENERIC_CONTAINERS(int),
                      RC_STRING_TYPES,
                      std::array<int, 100>>();
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
                auto seq = newshrink::towards(value, target);
                auto first = seq.next();
                RC_ASSERT(first);
                RC_ASSERT(*first == target);
            });

        templatedProp<T>(
            "tries an adjacent value last",
            [] (T target) {
                T value = *gen::distinctFrom(target);
                auto seq = newshrink::towards(value, target);
                auto fin = seq::last(seq);
                RC_ASSERT(fin);
                T diff = (value > target) ? (value - *fin) : (*fin - value);
                RC_ASSERT(diff == 1);
            });

        templatedProp<T>(
            "shrinking towards self yields empty shrink",
            [] (T target) {
                RC_ASSERT(!newshrink::towards(target, target).next());
            });
    }
};

} // namespace

TEST_CASE("newshrink::towards") {
    meta::forEachType<ShrinkTowardsProperties, RC_INTEGRAL_TYPES>();
}
