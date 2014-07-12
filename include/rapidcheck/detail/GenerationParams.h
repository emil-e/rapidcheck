#pragma once

namespace rc {
namespace detail {

class RandomEngine;
class RoseNode;

namespace param {

//! The current generation size.
struct Size {
    typedef int ValueType;
    static int defaultValue() { return 0; }
};

//! Disable shrinking for generators in scope.
struct NoShrink {
    typedef int ValueType;
    static int defaultValue() { return 0; }
};

//! The current random engine.
struct RandomEngine {
    typedef rc::detail::RandomEngine *ValueType;
    static rc::detail::RandomEngine *defaultValue() { return nullptr; }
};

//! The current `RoseNode`.
struct CurrentNode {
    typedef RoseNode *ValueType;
    static RoseNode *defaultValue() { return nullptr; }
};

} // namespace param
} // namespace detail
} // namespace rc
