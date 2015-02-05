#pragma once

#include "rapidcheck/gen/Common.h"

namespace rc {
namespace detail {

template<typename Callable, typename ReturnType, typename ...Args>
struct Invoker;

template<typename Callable, typename ReturnType>
struct Invoker<Callable, ReturnType>
{
    static ReturnType invoke(const Callable &callable) { return callable(); }
};

template<typename Callable,
         typename ReturnType,
         typename Arg,
         typename ...Args>
struct Invoker<Callable, ReturnType, Arg, Args...>
{
    static ReturnType
    invoke(const Callable &callable)
    {
        auto arg(*gen::arbitrary<typename std::decay<Arg>::type>());
        auto curried = [&] (Args &&...args) {
            return callable(std::move(arg), std::move(args)...);
        };
        return Invoker<decltype(curried), ReturnType, Args...>::invoke(curried);
    }
};

template<typename Functor, typename Ret, typename ...Args>
FunctorHelper<Ret (Functor::*)(Args...) const>::FunctorHelper(Functor functor)
    : m_functor(std::move(functor)) {}

template<typename Functor, typename Ret, typename ...Args>
Ret FunctorHelper<Ret (Functor::*)(Args...) const>::operator()() const
{ return Invoker<Functor, Ret, Args...>::invoke(m_functor); }

template<typename Callable>
Quantifier<Callable>::Quantifier(Callable callable)
    : FunctorHelper<decltype(&Callable::operator())>(std::move(callable)) {}

template<typename Ret, typename ...Args>
Quantifier<Ret (*)(Args...)>::Quantifier(Function function) : m_function(function) {}

template<typename Ret, typename ...Args>
Ret Quantifier<Ret (*)(Args...)>::operator()() const
{ return Invoker<Function, Ret, Args...>::invoke(m_function); }

} // namespace detail
} // namespace rc
