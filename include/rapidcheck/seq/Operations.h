#pragma once

#include "rapidcheck/Seq.h"

namespace rc {
namespace seq {

//! Returns the length of the given sequence. This is an O(n) operation.
template<typename T>
std::size_t length(Seq<T> seq);

} // namespace seq
} // namespace rc

#include "Operations.hpp"
