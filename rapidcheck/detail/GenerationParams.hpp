#pragma once

namespace rc {
namespace detail {
namespace param {

//! The current generation size.
struct Size { typedef size_t ValueType; };

//! Disable shrinking for generators in scope.
struct NoShrink { typedef int ValueType; };

} // namespace param
} // namespace detail
} // namespace rc
