#pragma once

namespace rc {
namespace gen {

template<typename Gen> class Resize;
template<typename Gen> class Scale;
template<typename Gen> class NoShrink;

//! Returns a version of the given generator that always uses the specified size.
//!
//! @param gen  The generator to wrap.
template<typename Gen>
Resize<Gen> resize(int size, Gen gen);

//! Returns a version of the given generator that scales the generation size
//! according to the given factory.
//!
//! @param gen  The generator to wrap.
template<typename Gen>
Scale<Gen> scale(double scale, Gen gen);

//! Returns a generator which wraps the given generator but does not try to
//! the argument or any of its sub generators.
template<typename Gen>
NoShrink<Gen> noShrink(Gen generator);

} // namespace gen
} // namespace rc

#include "Parameters.hpp"
