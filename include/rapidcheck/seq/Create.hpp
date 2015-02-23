#pragma once

#include "rapidcheck/seq/Transform.h"

namespace rc {
namespace seq {
namespace detail {

template<typename T>
class RepeatSeq
{
public:
    template<typename Arg>
    RepeatSeq(Arg &&arg) : m_value(std::forward<Arg>(arg)) {}

    Maybe<T> operator()()
    { return m_value; }

private:
    T m_value;
};

template<typename T, int N>
class JustSeq
{
public:
    template<typename ...Args>
    JustSeq(Args &&...args)
        : m_values{std::forward<Args>(args)...}
        , m_next(0) {}

    Maybe<T> operator()()
    {
        if (m_next >= N)
            return Nothing;
        return std::move(m_values[m_next++]);
    }

private:
    std::array<T, N> m_values;
    std::size_t m_next;
};


template<typename Container>
class ContainerSeq
{
public:
    typedef Decay<typename Container::value_type> T;

    template<typename ...Args,
             typename = typename std::enable_if<
                 std::is_constructible<Container, Args...>::value>::type>
    ContainerSeq(Args &&...args)
        : m_container(std::forward<Args>(args)...)
        , m_iterator(begin(m_container))
        , m_position(0) {}

    ContainerSeq(const ContainerSeq &other)
        : m_container(other.m_container)
        , m_iterator(begin(m_container))
        , m_position(other.m_position)
    { std::advance(m_iterator, m_position); }

    ContainerSeq &operator=(const ContainerSeq &rhs)
    {
        m_container = rhs.m_container;
        m_iterator = begin(m_container);
        m_position = rhs.m_position;
        std::advance(m_iterator, m_position);
        return *this;
    }

    Maybe<T> operator()()
    {
        if (m_iterator == end(m_container))
            return Nothing;
        m_position++;
        return Maybe<T>(std::move(*(m_iterator++)));
    }

private:
    Container m_container;
    typename Container::iterator m_iterator;
    std::size_t m_position;
};

template<typename T, typename Callable>
class IterateSeq
{
public:
    template<typename ValueArg, typename CallableArg>
    IterateSeq(ValueArg &&value, CallableArg &&iterate)
        : m_value(std::forward<ValueArg>(value))
        , m_iterate(std::forward<CallableArg>(iterate)) {}

    Maybe<T> operator()()
    {
        T value = m_value;
        m_value = m_iterate(std::move(m_value));
        return value;
    }

private:
    T m_value;
    Callable m_iterate;
};

} // namespace detail

template<typename T>
Seq<Decay<T>> repeat(T &&value)
{ return makeSeq<detail::RepeatSeq<Decay<T>>>(std::forward<T>(value)); }

template<typename T, typename ...Ts>
Seq<Decay<T>> just(T &&value, Ts &&...values)
{
    return makeSeq<detail::JustSeq<Decay<T>, sizeof...(Ts) + 1>>(
        std::forward<T>(value),
        std::forward<Ts>(values)...);
}

template<typename Container>
Seq<Decay<typename Decay<Container>::value_type>>
fromContainer(Container &&container)
{
    typedef Decay<Container> ContainerT;
    typedef Decay<typename ContainerT::value_type> T;

    if (container.empty())
        return Seq<T>();

    return makeSeq<detail::ContainerSeq<ContainerT>>(
        std::forward<Container>(container));
}

template<typename Iterator>
Seq<typename std::iterator_traits<Iterator>::value_type>
fromIteratorRange(Iterator start, Iterator end)
{
    typedef typename std::iterator_traits<Iterator>::value_type T;
    return makeSeq<detail::ContainerSeq<std::vector<T>>>(start, end);
}

template<typename T, typename Callable>
Seq<Decay<T>> iterate(T &&x, Callable &&f)
{
    return makeSeq<detail::IterateSeq<Decay<T>, Decay<Callable>>>(
        std::forward<T>(x),
        std::forward<Callable>(f));
}

template<typename T>
Seq<T> range(T start, T end)
{
    if (start == end)
        return Seq<T>();

    return seq::takeWhile(
        [=](T x) { return x != end; },
        (start < end) ? seq::iterate(start, [](int x) { return ++x; })
                      : seq::iterate(start, [](int x) { return --x; }));
}

Seq<std::size_t> index()
{
    return seq::iterate(static_cast<std::size_t>(0),
                        [](std::size_t i) { return i + 1; });
}

} // namespace seq
} // namespace rc
