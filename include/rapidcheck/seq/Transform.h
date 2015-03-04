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
template<typename T, typename Predicate>
Seq<T> dropWhile(Predicate &&pred, Seq<T> seq);

//! Takes elements until there is an element which does not match the predicate.
template<typename T, typename Predicate>
Seq<T> takeWhile(Predicate &&pred, Seq<T> seq);

//! Takes elements from the given `Seq`s and passes them as arguments to the
//! given callable and returns a `Seq` of such return values. For a single
//! `Seq`, this is regular "map" function but for more than one, it's what is
//! usually called "zipping with" a function. The length of the returned `Seq`
//! will be the length of the shortest `Seq` passed in.
//!
//! Fun fact: Also works with no sequences and in that case returns an infinite
//! sequence of the return values of calling the given callable.
template<typename ...Ts, typename Mapper>
Seq<typename std::result_of<Mapper(Ts...)>::type>
map(Mapper &&mapper, Seq<Ts> ...seqs);

//! Skips elements not matching the given predicate from the given stream.
template<typename T, typename Predicate>
Seq<T> filter(Predicate &&pred, Seq<T> seq);

//! Takes `Seq<Seq<T>>` and joins them together into a `Seq<T>`.
template<typename T>
Seq<T> join(Seq<Seq<T>> seqs);

//! Concatenates the given `Seq`s.
template<typename T, typename ...Ts>
Seq<T> concat(Seq<T> seq, Seq<Ts> ...seqs);

//! Maps each tuple elements of the given to `Seq`s to further `Seq`s and
//! concatenates them into one `Seq`. Sometimes called a "flat map".
template<typename ...Ts, typename Mapper>
Seq<typename std::result_of<Mapper(Ts...)>::type::ValueType>
mapcat(Mapper &&mapper, Seq<Ts> ...seqs);

//! Like `map` but expects the mapping functor to return a `Maybe`. If `Nothing`
//! is returned, the element is skipped. Otherwise, the `Maybe` is unwrapped and
//! included in the resulting `Seq`.
template<typename ...Ts, typename Mapper>
Seq<typename std::result_of<Mapper(Ts...)>::type::ValueType>
mapMaybe(Mapper &&mapper, Seq<Ts> ...seqs);

//! Creates a `Seq` which infinitely repeats the given `Seq`.
template<typename T>
Seq<T> cycle(Seq<T> seq);

//! Returns a `Seq` where each value of the given `Seq` is cast to the given
//! type using `static_cast`.
template<typename T, typename U>
Seq<T> cast(Seq<U> seq);

} // namespace seq
} // namespace rc

#include "Transform.hpp"
