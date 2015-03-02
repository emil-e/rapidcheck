#pragma once

#include "rapidcheck/fn/Common.h"

namespace rc {
namespace shrinkable {
namespace detail {

template<typename ValueCallable, typename ShrinksCallable>
class LambdaShrinkable
{
public:
    typedef Decay<typename std::result_of<ValueCallable()>::type> T;

    template<typename ValueArg, typename ShrinksArg>
    LambdaShrinkable(ValueArg &&value, ShrinksArg &&shrinks)
        : m_value(std::forward<ValueArg>(value))
        , m_shrinks(std::forward<ShrinksArg>(shrinks)) {}

    T value() const { return m_value(); }
    Seq<Shrinkable<T>> shrinks() const { return m_shrinks(); }

private:
    ValueCallable m_value;
    ShrinksCallable m_shrinks;
};

} // namespace detail

template<typename ValueCallable, typename ShrinksCallable>
Shrinkable<Decay<typename std::result_of<ValueCallable()>::type>>
lambda(ValueCallable &&value, ShrinksCallable &&shrinks)
{
    typedef detail::LambdaShrinkable<Decay<ValueCallable>, Decay<ShrinksCallable>>
        Impl;
    return makeShrinkable<Impl>(std::move(value), std::move(shrinks));
}

template<typename T, typename Value, typename>
Shrinkable<T> just(Value &&value, Seq<Shrinkable<T>> shrinks)
{
    return shrinkable::lambda(
        fn::constant(std::forward<T>(value)),
        fn::constant(std::move(shrinks)));
}

template<typename T>
Shrinkable<Decay<T>> just(T &&value)
{
    return shrinkable::just(std::forward<T>(value),
                            Seq<Shrinkable<Decay<T>>>());
}

} // namespace shrinkable
} // namespace rc
