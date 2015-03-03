#pragma once

#include "rapidcheck/Shrinkable.h"

namespace rc {
namespace shrinkable {

//! Maps the given shrinkable using the given mapping callable.
template<typename T, typename Mapper>
Shrinkable<typename std::result_of<Mapper(T)>::type>
map(Mapper &&mapper, Shrinkable<T> shrinkable);

} // namespace shrinkable
} // namespace rc

#include "Transform.hpp"
