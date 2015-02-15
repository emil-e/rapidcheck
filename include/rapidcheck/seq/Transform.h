#pragma once

#include "rapidcheck/Seq.h"

namespace rc {
namespace seq {

//! Drops the first `n` elements from the given `Seq`.
template<typename T>
Seq<T> drop(std::size_t n, Seq<T> seq);

} // namespace seq
} // namespace rc

#include "Transform.hpp"
