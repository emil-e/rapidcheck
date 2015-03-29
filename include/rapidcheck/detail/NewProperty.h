#pragma once

#include "rapidcheck/Gen.h"
#include "rapidcheck/detail/Results.h"

namespace rc {
namespace detail {

struct CaseDescription
{
    CaseResult result;
    Example example;
};

typedef Gen<CaseDescription> NewProperty;

//! Takes a callable and converts it into a generator of a `CaseResult` and a
//! counterexample. That is, a `NewProperty`.
template<typename Callable>
NewProperty toNewProperty(Callable &&callable);

} // namespace detail
} // namespace rc

#include "NewProperty.hpp"
