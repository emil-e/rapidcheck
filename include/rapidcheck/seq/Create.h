#pragma once

#include "rapidcheck/Seq.h"

namespace rc {
namespace seq {

//! Returns a `Seq` which returns just the given values.
template<typename T, typename ...Ts>
Seq<Decay<T>> just(T &&value, Ts &&...values);

//! Creates a sequence from the given STL-like container.
template<typename Container>
Seq<Decay<typename Decay<Container>::value_type>>
fromContainer(Container &&container);

//! Returns an infinite sequence by repeatedly applying the given callable to
//! the given value, i.e. [x, f(x), f(f(x))...].
template<typename T, typename Callable>
Seq<Decay<T>> iterate(T &&x, Callable &&f);

} // namespace seq
} // namespace rc

#include "Create.hpp"
