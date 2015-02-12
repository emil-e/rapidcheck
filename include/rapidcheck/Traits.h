#pragma once

#include <type_traits>

namespace rc {

//! Convenience wrapper over std::decay, shorter to type.
template<typename T>
using Decay = typename std::decay<T>::type;

} // namespace rc
