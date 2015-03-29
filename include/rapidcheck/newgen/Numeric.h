#pragma once

namespace rc {
namespace newgen {

//! Generates an integer in a given range.
//!
//! @param min  The minimum value, inclusive.
//! @param max  The maximum value, exclusive.
template<typename T>
Gen<T> inRange(T min, T max);

} // namespace newgen
} // namespace rc

#include "Numeric.hpp"
