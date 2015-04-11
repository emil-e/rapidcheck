#pragma once

#include "rapidcheck/detail/IntSequence.h"

// TODO DOCUMENT!

namespace rc {
namespace detail {

template<std::size_t I, typename T>
struct TupleElement {
    template<typename Arg>
    explicit TupleElement(Arg &&arg) : value(std::forward<Arg>(arg)) {}

    T value;
};

template<std::size_t I, typename T>
T &getElement(TupleElement<I, T> &element) { return element.value; }

template<std::size_t I, typename T>
const T &getElement(const TupleElement<I, T> &element) { return element.value; }

template<typename Indexes, typename ...Ts>
class TupleImpl;

template<std::size_t ...Is, typename ...Ts>
struct TupleImpl<IndexSequence<Is...>, Ts...>
    : public TupleElement<Is, Ts>...
{
    TupleImpl() = default;

    template<typename ...Args>
    TupleImpl(Args &&...args)
        : TupleElement<Is, Ts>(std::forward<Args>(args))... {}

    template<typename Callable>
    auto applyRef(Callable &&callable)
        -> decltype(callable(std::declval<Ts>()...))
    { return callable(getElement<Is>(*this)...); }

    template<typename Callable>
    auto applyRef(Callable &&callable) const
        -> decltype(callable(std::declval<Ts>()...))
    { return callable(getElement<Is>(*this)...); }

    template<typename Callable>
    auto applyRRef(Callable &&callable)
        -> decltype(callable(std::move(std::declval<Ts>())...))
    { return callable(std::move(getElement<Is>(*this))...); }
};

template<typename ...Ts>
class FastTuple
{
private:
    detail::TupleImpl<MakeIndexSequence<sizeof...(Ts)>, Ts...> m_impl;

public:
    template<typename Arg,
             typename ...Args,
             typename = typename std::enable_if<
                 !std::is_same<Decay<Arg>, FastTuple>::value>::type>
    FastTuple(Arg &&arg, Args &&...args)
        : m_impl(std::forward<Arg>(arg), std::forward<Args>(args)...) {}

    template<std::size_t I>
    auto at() -> decltype(detail::getElement<I>(m_impl))
    { return detail::getElement<I>(m_impl); }

    template<std::size_t I>
    auto at() const -> decltype(detail::getElement<I>(m_impl))
    { return detail::getElement<I>(m_impl); }

    template<typename Callable>
    auto applyRef(Callable &&callable)
        -> decltype(callable(std::declval<Ts>()...))
    { return m_impl.applyRef(std::forward<Callable>(callable)); }

    template<typename Callable>
    auto applyRef(Callable &&callable) const
        -> decltype(callable(std::declval<Ts>()...))
    { return m_impl.applyRef(std::forward<Callable>(callable)); }

    template<typename Callable>
    auto applyRRef(Callable &&callable)
        -> decltype(callable(std::move(std::declval<Ts>())...))
    { return m_impl.applyRRef(std::forward<Callable>(callable)); }
};

template<>
class FastTuple<>
{
public:
    template<typename Callable>
    auto applyRef(Callable &&callable) const
        -> decltype(callable())
    { return callable(); }

    template<typename Callable>
    auto applyRRef(Callable &&callable)
        -> decltype(callable())
    { return callable(); }
};

template<typename ...Ts, typename Callable>
auto applyTuple(FastTuple<Ts...> &tuple, Callable &&callable)
    -> decltype(tuple.applyRef(std::forward<Callable>(callable)))
{ return tuple.applyRef(std::forward<Callable>(callable)); }

template<typename ...Ts, typename Callable>
auto applyTuple(const FastTuple<Ts...> &tuple, Callable &&callable)
    -> decltype(tuple.applyRef(std::forward<Callable>(callable)))
{ return tuple.applyRef(std::forward<Callable>(callable)); }

template<typename ...Ts, typename Callable>
auto applyTuple(FastTuple<Ts...> &&tuple, Callable &&callable)
    -> decltype(tuple.applyRRef(std::forward<Callable>(callable)))
{ return tuple.applyRRef(std::forward<Callable>(callable)); }

} // namespace detail
} // namespace rc
