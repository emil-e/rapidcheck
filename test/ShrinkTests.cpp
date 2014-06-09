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

DESCRIBE("shrink::sequentially") {
    // TODO test emtpy shrink but not with property

    it("joins shrinkers together",
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

DESCRIBE("shrink::unfold") {
    it("mimics a for loop",
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

           for (int i = start; i < end; i += incr) {
               ASSERT_THAT(i != iterator->next());
           }
           ASSERT_THAT(!iterator->hasNext());
       });
}
