#pragma once

#include "rapidcheck/Generator.h"

namespace rc {
namespace detail {

template<typename Functor, typename Ret, typename ...Args>
FunctorHelper<Ret (Functor::*)(Args...) const>::FunctorHelper(Functor functor)
    : m_functor(std::move(functor)) {}

template<typename Functor, typename Ret, typename ...Args>
Ret FunctorHelper<Ret (Functor::*)(Args...) const>::operator()() const
{ return m_functor(pick<typename std::decay<Args>::type>()...); }

template<typename Callable>
Quantifier<Callable>::Quantifier(Callable callable)
    : FunctorHelper<decltype(&Callable::operator())>(std::move(callable)) {}

template<typename Ret, typename ...Args>
Quantifier<Ret (*)(Args...)>::Quantifier(Function function) : m_function(function) {}

template<typename Ret, typename ...Args>
Ret Quantifier<Ret (*)(Args...)>::operator()() const
{ return m_function(pick<typename std::decay<Args>::type>()...); }

} // namespace detail
} // namespace rc
