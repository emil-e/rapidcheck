#pragma once

namespace rc {
namespace newgen {

//! Generator of text characters. Common occuring characters have a higher
//! probability of being generated.
template<typename T>
Gen<T> character();

} // namespace newgen
} // namespace rc

#include "Text.hpp"
