#pragma once

#include "rapidcheck/seq/Operations.h"

namespace rc {
namespace shrinkable {

template <typename T, typename Predicate>
bool all(const Shrinkable<T> &shrinkable, Predicate predicate) {
  if (!predicate(shrinkable)) {
    return false;
  }

  return seq::all(shrinkable.shrinks(),
                  [=](const Shrinkable<T> &shrink) {
                    return shrinkable::all(shrink, predicate);
                  });
}

template <typename Predicate, typename T>
std::pair<T, int> findLocalMin(const Shrinkable<T> &shrinkable,
                               Predicate pred) {
  std::pair<T, int> result(shrinkable.value(), 0);
  Seq<Shrinkable<T>> shrinks = shrinkable.shrinks();
  Maybe<Shrinkable<T>> shrink;
  while ((shrink = shrinks.next())) {
    T value = shrink->value();
    if (pred(value)) {
      result.first = std::move(value);
      shrinks = shrink->shrinks();
      result.second++;
    }
  }

  return result;
}

template <typename T>
Seq<T> immediateShrinks(const Shrinkable<T> &shrinkable) {
  return seq::map(shrinkable.shrinks(),
                  [](const Shrinkable<T> &shrink) { return shrink.value(); });
}

} // namespace shrinkable
} // namespace rc
