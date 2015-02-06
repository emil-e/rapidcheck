#include <catch.hpp>
#include <rapidcheck-catch.h>

#include <array>

#include "util/TypeListMacros.h"
#include "util/Util.h"
#include "util/Meta.h"

using namespace rc;

TEST_CASE("shrink::sequentially") {
    prop("joins shrinkers together",
         [] (const std::vector<int> &xs) {
             int split = xs.empty() ? 0 : *gen::ranged<size_t>(0, xs.size());
             shrink::IteratorUP<int> it(
                 shrink::sequentially(
                     shrink::constant<int>({xs.begin(), xs.begin() + split}),
                     shrink::constant<int>({xs.begin() + split, xs.end()})));

             RC_ASSERT(takeAll(it) == xs);
         });
}

TEST_CASE("shrink::unfold") {
    prop("mimics a for loop",
         [] {
             int start = *gen::arbitrary<int>();
             int end = *gen::suchThat<int>([=](int x){ return x > start; });
             int incr = *gen::ranged<int>(1, end - start + 1);
             auto iterator = shrink::unfold(
                 start,
                 [=](int x) { return x < end; },
                 [=](int x) { return std::make_pair(x, x + incr); });

             for (int i = start; i < end; i += incr)
                 RC_ASSERT(i == iterator->next());
             RC_ASSERT(!iterator->hasNext());
         });
}

TEST_CASE("shrink::map") {
    prop("maps values from one type to another",
         [] (const std::vector<int> &xs) {
             auto it = shrink::map(
                 shrink::constant(xs),
                 [](int x){ return std::to_string(x); });
             for (int e : xs)
                 RC_ASSERT(std::to_string(e) == it->next());
             RC_ASSERT(!it->hasNext());
         });
}

TEST_CASE("shrink::constant") {
    prop("yields constants in order",
         [] (const std::vector<int> &xs) {
             auto it = shrink::constant(xs);
             for (int e : xs)
                 RC_ASSERT(e == it->next());
             RC_ASSERT(!it->hasNext());
         });
}

TEST_CASE("shrink::nothing") {
    SECTION("never has next shrink") {
        REQUIRE_FALSE(shrink::nothing<int>()->hasNext());
    }
}


struct EachElementProperties
{
    template<typename T>
    static void exec()
    {
        typedef DeepDecayT<typename T::value_type> Element;
        static auto smallValue = gen::scale(0.1, gen::arbitrary<Element>());
        static auto smallValues = gen::collection<T>(smallValue);

        templatedProp<T>(
            "the container size stays the same",
            [&] {
                auto elements = *smallValues;
                auto it = shrink::eachElement(
                    elements,
                    [&] (const Element &x) { return smallValue.shrink(x); });

                auto size = containerSize(elements);
                while (it->hasNext()) {
                    RC_ASSERT(containerSize(it->next()) == size);
                }
            });

        TEMPLATED_SECTION(T, "has no shrinks for empty collections") {
            auto it = shrink::eachElement(
                T(),
                [] (const Element &x) {
                    return gen::arbitrary<Element>().shrink(x);
                });
            REQUIRE(!it->hasNext());
        }

        templatedProp<T>(
            "has no shrinks if elements have no shrinks",
            [] {
                auto it = shrink::eachElement(
                    *smallValues,
                    [] (const Element &x) {
                        return shrink::nothing<Element>();
                    });
                RC_ASSERT(!it->hasNext());
            });

        templatedProp<T>(
            "the number of shrinks is never greater than the sum of the "
            "shrink counts for the iterators of the elements",
            [] {
                auto elements = *smallValues;
                auto it = shrink::eachElement(
                    elements,
                    [&] (const Element &x) { return smallValue.shrink(x); });

                std::size_t count = 0;
                for (const auto &element : elements)
                    count += shrinkCount(smallValue.shrink(element));
                RC_ASSERT(shrinkCount(it) <= count);
            });

        templatedProp<T>(
            "for every shrink, a value is replaced with one of its possible "
            "shrinks",
            [] {
                auto elements = *smallValues;
                auto it = shrink::eachElement(
                    elements,
                    [&] (const Element &x) { return smallValue.shrink(x); });

                while (it->hasNext()) {
                    auto shrunk = it->next();
                    auto added = setDifference<Element>(shrunk, elements);
                    auto removed = setDifference<Element>(elements, shrunk);
                    RC_ASSERT(added.size() == 1);
                    RC_ASSERT(removed.size() == 1);
                    RC_ASSERT(hasShrink(smallValue.shrink(removed[0]), added[0]));
                }
            });
    }
};

TEST_CASE("shrink::eachElement") {
    meta::forEachType<EachElementProperties,
                      RC_GENERIC_CONTAINERS(int),
                      RC_STRING_TYPES,
                      std::array<int, 100>>();
}

struct RemoveChunksProperties
{
    template<typename T>
    static void exec()
    {
        typedef DeepDecayT<typename T::value_type> Element;
        static auto smallValue =
            gen::scale(0.1, gen::arbitrary<typename T::value_type>());
        static auto smallValues = gen::collection<T>(smallValue);
        static auto fewSmallValues = gen::scale(0.25, smallValues);

        templatedProp<T>(
            "first tries empty collection",
            [] {
                RC_ASSERT(shrink::removeChunks(*smallValues)->next().empty());
            });

        templatedProp<T>(
            "successively increases in size for each shrink",
            [] {
                auto it = shrink::removeChunks(*fewSmallValues);
                T c;
                while (it->hasNext()) {
                    auto next = it->next();
                    RC_ASSERT(containerSize(next) >= containerSize(c));
                    c = std::move(next);
                }
            });

        templatedProp<T>(
            "shrinks to a subset of the original",
            [] {
                auto elements = *fewSmallValues;
                auto it = shrink::removeChunks(elements);
                while (it->hasNext()) {
                    auto diff(setDifference<Element>(it->next(), elements));
                    RC_ASSERT(diff.size() == 0);
                }
            });

        templatedProp<T>(
            "every removal of consecutive elements is a possible shrink",
            [] {
                // TODO non-empty generator
                auto elements =  *gen::suchThat(
                    fewSmallValues,
                    [] (const T &x) {
                        return std::distance(begin(x), end(x)) != 0;
                    });
                auto size = containerSize(elements);
                int begin = *gen::ranged<int>(0, size - 1);
                int end = *gen::ranged<int>(begin + 1, size);

                detail::CollectionBuilder<T> builder;
                int i = 0;
                for (const auto &x : elements) {
                    if ((i < begin) && (i >= end))
                        builder.add(x);
                    i++;
                }

                auto it = shrink::removeChunks(elements);
                RC_ASSERT(hasShrink(it, builder.result()));
            });

        templatedProp<T>(
            "never yields the original value",
            [] {
                auto elements = *fewSmallValues;
                auto it = shrink::removeChunks(elements);
                RC_ASSERT(!hasShrink(it, elements));
            });
    }
};

TEST_CASE("shrink::removeChunks") {
    meta::forEachType<RemoveChunksProperties,
                      RC_GENERIC_CONTAINERS(int),
                      std::string,
                      std::wstring>();
}

struct ShrinkTowardsProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "first tries target immediately",
            [] (T target) {
                T value = *gen::suchThat(
                    gen::arbitrary<T>(),
                    [=] (T x) { return x != target; });
                auto it = shrink::towards(value, target);
                RC_ASSERT(it->hasNext());
                RC_ASSERT(it->next() == target);
            });

        templatedProp<T>(
            "tries an adjacent value last",
            [] (T target) {
                T value = *gen::suchThat(
                    gen::arbitrary<T>(),
                    [=] (T x) { return x != target; });
                auto it = shrink::towards(value, target);
                T fin = finalShrink(it);
                T diff = (value > target) ? (value - fin) : (fin - value);
                RC_ASSERT(diff == 1);
            });

        templatedProp<T>(
            "shrinking towards self yields empty shrink",
            [] (T target) {
                RC_ASSERT(!shrink::towards(target, target)->hasNext());
            });
    }
};

TEST_CASE("shrink::towards") {
    meta::forEachType<ShrinkTowardsProperties, RC_INTEGRAL_TYPES>();
}

TEST_CASE("shrink::filter") {
    prop("if the predicate always returns true, yields the same as the original",
         [] (const std::vector<int> &shrinks) {
             auto it = shrink::filter(shrink::constant(shrinks),
                                      [] (int x) { return true; });
             RC_ASSERT(takeAll(it) == shrinks);
         });

    prop("if the predicate always returns false, yields nothing",
         [] (const std::vector<std::string> &shrinks) {
             auto it = shrink::filter(shrink::constant(shrinks),
                                      [] (const std::string &x) { return false; });
             RC_ASSERT(!it->hasNext());
         });

    prop("never returns an item which does not satisfy the predicate",
         [] (const std::vector<int> &shrinks) {
             auto max = *gen::positive<int>();
             auto it = shrink::filter(shrink::constant(shrinks),
                                      [=] (int x) { return x < max; });
             while (it->hasNext())
                 RC_ASSERT(it->next() < max);
         });
}
