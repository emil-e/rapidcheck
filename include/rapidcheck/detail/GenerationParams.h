#pragma once

#include "rapidcheck/Random.h"

namespace rc {
namespace detail {

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
struct Random {
    typedef rc::Random ValueType;
    static rc::Random defaultValue() { return rc::Random({0, 0, 0, 0}); }
};

//! The current `RoseNode`.
struct CurrentNode {
    typedef RoseNode *ValueType;
    static RoseNode *defaultValue() { return nullptr; }
};

} // namespace param
} // namespace detail
} // namespace rc
