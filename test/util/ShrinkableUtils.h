#pragma once

namespace rc {
namespace test {

//! Returns the item at `index` or the last element if the index is out of
//! bounds. If the `Seq` is empty, returns `Nothing`.
template<typename T>
Maybe<T> atOrLast(Seq<T> seq, std::size_t index)
{
    Maybe<T> prev;
    Maybe<T> x;
    std::size_t n = index;
    while ((x = seq.next())) {
        if (n-- == 0)
            return x;
        prev = std::move(x);
    }

    return prev;
}

//! Calls `assertion` with a value and some shrink of the value when going down
//! arbitrary paths into the shrinkable tree. `RC_ASSERT` in this funciton to do
//! something useful.
template<typename T, typename Assertion>
void onAnyPath(const Shrinkable<T> &shrinkable, Assertion assertion)
{
    const auto path = *gen::collection<std::vector<int>>(
        gen::ranged<std::size_t>(0, 100));
    Shrinkable<T> current = shrinkable;
    for (const auto n : path) {
        Maybe<Shrinkable<T>> shrink = atOrLast(current.shrinks(), n);
        if (!shrink)
            return;
        assertion(current, *shrink);
        current = std::move(*shrink);
    }
}

} // namespace test
} // namespace rc
