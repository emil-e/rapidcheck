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

} // namespace shrinkable
} // namespace rc

#include "Transform.hpp"
