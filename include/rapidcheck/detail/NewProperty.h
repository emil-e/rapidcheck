#pragma once

#include "rapidcheck/Gen.h"
#include "rapidcheck/detail/Results.h"

namespace rc {
namespace detail {

typedef Gen<std::pair<CaseResult, Counterexample>> NewProperty;

//! Takes a callable and converts it into a generator of a `CaseResult` and a
//! counterexample. That is, a `NewProperty`.
template<typename Callable>
NewProperty toNewProperty(Callable &&callable);

} // namespace detail
} // namespace rc

#include "NewProperty.hpp"
