#pragma once

namespace rc {
namespace gen {

template<typename T> class Ranged;
template<typename T> class NonZero;
template<typename T> class Positive;
template<typename T> class Negative;
template<typename T> class NonNegative;

//! Generates an arbitrary value between \c min and \c max. Both \c min and
//! \c max are included in the range.
//!
//! @param min  The minimum value.
//! @param max  The maximum value.
template<typename T>
Ranged<T> ranged(T min, T max);

//! Generates a non-zero value of type `T`.
//!
//! @tparam T  An integral type.
template<typename T>
NonZero<T> nonZero();

//! Generates a positive value (> 0) of type `T`.
//!
//! @tparam T  An integral type.
template<typename T>
Positive<T> positive();

//! Generates a negative (< 0) value of type `T`.
//!
//! @tparam T  An integral type.
template<typename T>
Negative<T> negative();

//! Generates a non-negative (>= 0) value of type `T`.
//!
//! @tparam T  An integral type.
template<typename T>
NonNegative<T> nonNegative();

} // namespace gen
} // namespace rc

#include "Numeric.hpp"
