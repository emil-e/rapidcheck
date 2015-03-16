#pragma once

#include "rapidcheck/detail/FunctionTraits.h"
#include "rapidcheck/newgen/detail/Recipe.h"

namespace rc {
namespace newgen {
namespace detail {

//! "Raw" version of `gen::exec` (which uses this generator) that both return
//! the generated value and the `Recipe` used to do so.
template<typename Callable>
Gen<std::pair<Decay<rc::detail::ReturnType<Callable>>, Recipe>>
execRaw(Callable callable);

} // namespace detail
} // namespace newgen
} // namespace rc

#include "ExecRaw.hpp"
