#pragma once

namespace rc {
namespace gen {

//! Generates an integer in a given range.
//!
//! @param min  The minimum value, inclusive.
//! @param max  The maximum value, exclusive.
template<typename T>
Gen<T> inRange(T min, T max);

//! Generates a value that is not equal to `0`.
template<typename T>
Gen<T> nonZero();

//! Generates a value which is greater than `0`.
template<typename T>
Gen<T> positive();

//! Generates a value which is less than `0`.
template<typename T>
Gen<T> negative();

//! Generates a value which is not less than `0`.
template<typename T>
Gen<T> nonNegative();

} // namespace gen
} // namespace rc

#include "Numeric.hpp"
