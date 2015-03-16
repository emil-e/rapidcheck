#pragma once

#include "TypeList.h"

namespace rc {
namespace detail {

template<typename T>
struct MemberFunctionTraitsImpl;

template<typename T, typename Ret, typename ...Args>
struct MemberFunctionTraitsImpl<Ret (T::*)(Args...)>
{
    typedef Ret ReturnType;
    typedef TypeList<Args...> ArgTypes;
    typedef std::function<ReturnType(Args...)> StdFunction;
    typedef T Target;
};

template<typename T, typename Ret, typename ...Args>
struct MemberFunctionTraitsImpl<Ret (T::*)(Args...) const>
{
    typedef Ret ReturnType;
    typedef TypeList<Args...> ArgTypes;
    typedef std::function<ReturnType(Args...)> StdFunction;
    typedef T Target;
};

template<typename T>
struct FunctionTraitsImpl
    : public MemberFunctionTraitsImpl<decltype(&T::operator())> {};

// NOTE: Base template is for functors.

template<typename Ret, typename ...Args>
struct FunctionTraitsImpl<Ret (*)(Args...)>
{
    typedef Ret ReturnType;
    typedef TypeList<Args...> ArgTypes;
    typedef std::function<ReturnType(Args...)> StdFunction;
};


//! Traits class for capturing various information about a callable object.
template<typename T>
using FunctionTraits = FunctionTraitsImpl<Decay<T>>;

//! The return type of a function.
template<typename T>
using ReturnType = typename FunctionTraits<T>::ReturnType;

//! `TypeList` of argument types for `T`.
template<typename T>
using ArgTypes = typename FunctionTraits<T>::ArgTypes;

//1 The std::function type that can be created from `T`.
template<typename T>
using StdFunction = typename FunctionTraits<T>::StdFunction;

} // namespace detail
} // namespace rc
