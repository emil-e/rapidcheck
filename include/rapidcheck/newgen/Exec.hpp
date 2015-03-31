#pragma once

#include "rapidcheck/newgen/detail/Recipe.h"
#include "rapidcheck/newgen/detail/ExecRaw.h"

namespace rc {
namespace newgen {

template<typename Callable>
Gen<Decay<rc::detail::ReturnType<Callable>>> exec(Callable &&callable)
{
    using namespace detail;
    typedef rc::detail::ReturnType<Callable> T;

    return newgen::map(
        execRaw(std::forward<Callable>(callable)),
        [](std::pair<T, Recipe> &&p) { return std::move(p.first); });
}

} // namespace newgen
} // namespace rc
