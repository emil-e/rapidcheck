#pragma once

#include "rapidcheck/detail/FastTuple.h"

namespace rc {
namespace detail {

//! TMP utility for passing around a list of types.
template<typename ...Types>
struct TypeList;

namespace tl {

template<typename T>
struct ToTupleImpl;

template<typename ...Types>
struct ToTupleImpl<TypeList<Types...>>
{
    typedef std::tuple<Types...> Type;
};

//! Turns a `TypeList` into am `std::tuple`.
template<typename T>
using ToTuple = typename ToTupleImpl<T>::Type;

template<typename T>
struct ToFastTupleImpl;

template<typename ...Types>
struct ToFastTupleImpl<TypeList<Types...>>
{
    typedef rc::detail::FastTuple<Types...> Type;
};

//! Turns a `TypeList` into am `std::tuple`.
template<typename T>
using ToFastTuple = typename ToFastTupleImpl<T>::Type;

template<typename T>
struct DecayAllImpl;

template<typename ...Types>
struct DecayAllImpl<TypeList<Types...>>
{
    typedef TypeList<typename std::decay<Types>::type...> Type;
};

//! Decays all the types in a `TypeList`.
template<typename T>
using DecayAll = typename DecayAllImpl<T>::Type;

} // namespace tl
} // namespace detail
} // namespace rc
