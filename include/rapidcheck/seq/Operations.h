#pragma once

#include "rapidcheck/Seq.h"

namespace rc {
namespace seq {

//! Returns the length of the given sequence. This is an O(n) operation.
template<typename T>
std::size_t length(Seq<T> seq);

//! Calls the given callable once for each element of the sequence.
template<typename T, typename Callable>
void forEach(Seq<T> seq, Callable callable);

} // namespace seq
} // namespace rc

#include "Operations.hpp"
