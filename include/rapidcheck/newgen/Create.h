#pragma once

#include "rapidcheck/Gen.h"

namespace rc {
namespace newgen {

//! Returns a generator which always returns the given value with no shrinks.
template<typename T>
Gen<Decay<T>> just(T &&value);

} // namespace newgen
} // namespace rc

#include "Create.hpp"
