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

inline std::ostream &operator<<(std::ostream &os, const CaseDescription &desc);

typedef Gen<CaseDescription> Property;

/// Takes a callable and converts it into a generator of a `CaseResult` and a
/// counterexample. That is, a `Property`.
template<typename Callable>
Property toProperty(Callable &&callable);

} // namespace detail
} // namespace rc

#include "Property.hpp"
