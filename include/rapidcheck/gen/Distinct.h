#pragma once

namespace rc {
namespace gen {

/// Generates a value using the given generator that is not equal to the given
/// value.
template<typename T, typename Value>
Gen<T> distinctFrom(Gen<T> gen, Value &&value);

/// Generates an arbitrary value using the given generator that is not equal to
/// the given value.
template<typename Value>
Gen<Decay<Value>> distinctFrom(Value &&value);

} // namespace gen
} // namespace rc

#include "Distinct.hpp"
