#pragma once

#include "rapidcheck/seq/Operations.h"

namespace rc {
namespace shrinkable {

template<typename T, typename Predicate>
bool all(const Shrinkable<T> &shrinkable, Predicate predicate)
{
    if (!predicate(shrinkable))
        return false;

    return seq::all(shrinkable.shrinks(), [=](const Shrinkable<T> &shrink) {
        return shrinkable::all(shrink, predicate);
    });
}

} // namespace shrinkable
} // namespace rc
