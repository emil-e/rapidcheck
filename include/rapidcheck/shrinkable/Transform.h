#pragma once

#include "rapidcheck/Shrinkable.h"

namespace rc {
namespace shrinkable {

//! Maps the given shrinkable using the given mapping callable.
template<typename T, typename Mapper>
Shrinkable<typename std::result_of<Mapper(T)>::type>
map(Mapper &&mapper, Shrinkable<T> shrinkable);

//! Returns a shrinkable equal to the given shrinkable but with the shrinks
//! (lazily) mapped by the given mapping callable. Since the value is not mapped
//! also the output type is the same as the output type.
template<typename T, typename Mapper>
Shrinkable<T> mapShrinks(Mapper &&mapper, Shrinkable<T> shrinkable);

//! Recursively filters the given shrinkable using the given predicate. Any
//! subtree with a root for which the predicate returns false is discarded,
//! including the passed in root which is why this function returns a `Maybe`.
template<typename T, typename Predicate>
Maybe<Shrinkable<T>> filter(Predicate pred, Shrinkable<T> shrinkable);

//! Given two `Shrinkables`, returns a `Shrinkable` pair that first shrinks the
//! first element and then the second.
template<typename T1, typename T2>
Shrinkable<std::pair<T1, T2>> pair(Shrinkable<T1> s1, Shrinkable<T2> s2);

} // namespace shrinkable
} // namespace rc

#include "Transform.hpp"
