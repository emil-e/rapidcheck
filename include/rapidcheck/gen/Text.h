#pragma once

namespace rc {
namespace gen {

//! Generator of text characters. Common occuring characters have a higher
//! probability of being generated.
template<typename T>
Gen<T> character();

} // namespace gen
} // namespace rc

#include "Text.hpp"
