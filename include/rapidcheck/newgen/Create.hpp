#pragma once

#include "rapidcheck/fn/Common.h"
#include "rapidcheck/shrinkable/Create.h"

namespace rc {
namespace newgen {

template<typename T>
Gen<Decay<T>> just(T &&value)
{ return fn::constant(shrinkable::just(std::forward<T>(value))); }

} // namespace newgen
} // namespace rc
