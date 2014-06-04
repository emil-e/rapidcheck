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

DESCRIBE("shrink::Sequentially") {
    it("joins shrinkers together",
       [] (const std::vector<int> &xs)
       {
           int split = pick(gen::ranged<size_t>(0, xs.size() - 1));
           shrink::IteratorUP<int> it(
               shrink::sequentially(
                   shrink::constant(std::vector<int>{xs.begin(), xs.begin() + split}),
                   shrink::constant(std::vector<int>{xs.begin() + split, xs.end()})));

           return takeAll(it) == xs;
       });
}
