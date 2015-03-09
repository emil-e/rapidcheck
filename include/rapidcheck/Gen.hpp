#pragma once

#include <cassert>

#include "rapidcheck/detail/Any.h"
#include "rapidcheck/detail/ImplicitParam.h"
#include "rapidcheck/newgen/Transform.h"
#include "rapidcheck/newgen/detail/GenerationHandler.h"

namespace rc {

template<typename T>
template<typename Impl, typename>
Gen<T>::Gen(Impl &&impl)
    : m_impl(std::forward<Impl>(impl))
{
    assert(m_impl);
}

template<typename T>
Shrinkable<T> Gen<T>::operator()(const Random &random, int size) const
{ return m_impl(random, size); }

template<typename T>
T Gen<T>::operator*() const
{
    using namespace detail;
    using rc::newgen::detail::param::CurrentHandler;
    const auto handler = ImplicitParam<CurrentHandler>::value();
    return std::move(
        handler->onGenerate(newgen::map(&Any::of<T>, *this)).template get<T>());
}

} // namespace rc
