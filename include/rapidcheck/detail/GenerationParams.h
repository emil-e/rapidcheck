#pragma once

namespace rc {
namespace detail {

class RandomEngine;
class RoseNode;

namespace param {

//! The current generation size.
struct Size { typedef int ValueType; };

//! Disable shrinking for generators in scope.
struct NoShrink { typedef int ValueType; };

//! The current random engine.
struct RandomEngine { typedef rc::detail::RandomEngine ValueType; };

//! The current `RoseNode`.
struct CurrentNode { typedef RoseNode *ValueType; };

} // namespace param
} // namespace detail
} // namespace rc
