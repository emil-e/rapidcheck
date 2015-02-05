#pragma once

namespace rc {
namespace gen {

template<typename T> class Character;

//! Generates a character of type `T`.
//!
//! @tparam T  The character type (i.e. char, wchar_t etc.)
template<typename T>
Character<T> character();

} // namespace gen
} // namespace rc

#include "Text.hpp"
