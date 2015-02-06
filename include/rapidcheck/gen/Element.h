#pragma once

#include "rapidcheck/detail/Traits.h"

namespace rc {
namespace gen {

template<typename Container> class ElementOf;

//! Returns a generator which randomly selects from the given container. The
//! container must support `begin` and `end`. Its iterator must meet the
//! requirements of BidirectionalIterator.
template<typename Container>
ElementOf<detail::DecayT<Container>> elementOf(Container &&container);

} // namespace gen
} // namespace rc

#include "Element.hpp"
