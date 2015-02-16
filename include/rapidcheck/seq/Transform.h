#pragma once

#include "rapidcheck/Seq.h"

namespace rc {
namespace seq {

//! Drops the first `n` elements from the given `Seq`.
template<typename T>
Seq<T> drop(std::size_t n, Seq<T> seq);

//! Takes the first `n` elements from the given `Seq`.
template<typename T>
Seq<T> take(std::size_t n, Seq<T> seq);

//! Drops all elements until the given predicate returns true.
template<typename Predicate, typename T>
Seq<T> dropWhile(Predicate &&pred, Seq<T> seq);

//! Takes elements until there is an element which does not match the predicate.
template<typename Predicate, typename T>
Seq<T> takeWhile(Predicate &&pred, Seq<T> seq);

} // namespace seq
} // namespace rc

#include "Transform.hpp"
