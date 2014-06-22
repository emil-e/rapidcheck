#include <catch.hpp>
#include <rapidcheck.h>

#include "Utils.h"
#include "Meta.h"

using namespace rc;

TEST_CASE("shrink::sequentially") {
    prop("joins shrinkers together",
         [] (const std::vector<int> &xs) {
             int split = xs.empty() ? 0 : pick(gen::ranged<size_t>(0, xs.size()));
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
             int start = pick<int>();
             int end = pick(gen::suchThat<int>([=](int x){ return x > start; }));
             int incr = pick(gen::ranged<int>(1, end - start + 1));
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


TEST_CASE("shrink::eachElement") {
    prop("tries shrinking one element at a time",
         [] {
             size_t size = pick(gen::ranged<size_t>(0, gen::currentSize() + 1));
             std::vector<int> elements(size, 2);

             // Two shrinks from each element, first 0 and then 1
             auto it = shrink::eachElement(
                 elements,
                 [](int x) { return shrink::constant<int>({0, 1}); });

             // This means that the number of shrinks should be size * 2
             for (size_t count = 0; count < (size * 2); count++) {
                 auto shrunk = it->next();
                 // Check each element
                 for (int i = 0; i < shrunk.size(); i++) {
                     // The current shrunk element is count / 2 since each
                     // element is shrunk twice
                     if (i == (count / 2)) {
                         // The shrunk element is first 0 and then 1
                         RC_ASSERT(shrunk[i] == (count % 2));
                     } else {
                         // The non-shrunk elements are always 2
                         RC_ASSERT(shrunk[i] == 2);
                     }
                 }
             }

             // Now we should be out of shrinks
             RC_ASSERT(!it->hasNext());
         });
}

TEST_CASE("shrink::removeChunks") {
    prop("first tries empty collection",
         [] (const std::vector<int> &elements) {
             RC_ASSERT(shrink::removeChunks(elements)->next().empty());
         });

    prop("successively increases size",
         [] (const std::vector<int> &elements) {
             auto it = shrink::removeChunks(elements);
             std::vector<int> c;
             while (it->hasNext()) {
                 auto next = it->next();
                 RC_ASSERT(next.size() >= c.size());
             }
         });

    prop("shrinks to a subset of the original",
         [] (const std::vector<int> &elements) {
             auto it = shrink::removeChunks(elements);
             while (it->hasNext()) {
                 auto shrunk = it->next();
                 for (int x : shrunk) {
                     auto result = std::find(elements.begin(), elements.end(), x);
                     RC_ASSERT(result != elements.end());
                 }
             }
         });
}

struct ShrinkTowardsProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "first tries target immediately",
            [] (T target) {
                T value = pick(gen::suchThat(
                                   gen::arbitrary<T>(),
                                   [=] (T x) { return x != target; }));
                auto it = shrink::towards(value, target);
                RC_ASSERT(it->hasNext());
                RC_ASSERT(it->next() == target);
            });

        templatedProp<T>(
            "tries an adjacent value last",
            [] (T target) {
                T value = pick(gen::suchThat(
                                   gen::arbitrary<T>(),
                                   [=] (T x) { return x != target; }));
                auto it = shrink::towards(value, target);
                T fin = finalShrink(it);
                T diff = (value > target) ? (value - fin) : (fin - value);
                RC_ASSERT(diff == 1);
            });
    }
};

TEST_CASE("shrink::towards") {
    meta::forEachType<ShrinkTowardsProperties, RC_INTEGRAL_TYPES>();
}
