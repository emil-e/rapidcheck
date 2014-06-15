#include <catch.hpp>
#include <rapidcheck.h>

using namespace rc;

template<typename T>
std::vector<T> takeAll(shrink::IteratorUP<T> &iterator)
{
    std::vector<T> items;
    while (iterator->hasNext())
        items.push_back(iterator->next());
    return items;
}

TEST_CASE("shrink::sequentially") {
    prop("joins shrinkers together",
         [] (const std::vector<int> &xs)
         {
             int split = xs.empty() ? 0 : pick(gen::ranged<size_t>(0, xs.size()));
             shrink::IteratorUP<int> it(
                 shrink::sequentially(
                     shrink::constant(std::vector<int>{xs.begin(), xs.begin() + split}),
                     shrink::constant(std::vector<int>{xs.begin() + split, xs.end()})));

             return takeAll(it) == xs;
         });
}

TEST_CASE("shrink::unfold") {
    prop("mimics a for loop",
         []
         {
             int start = pick<int>();
             int end = pick(gen::suchThat(gen::arbitrary<int>(),
                                          [=](int x){ return x > start; }));
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
         [] (const std::vector<int> &xs)
         {
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
         [] (const std::vector<int> &xs)
         {
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
