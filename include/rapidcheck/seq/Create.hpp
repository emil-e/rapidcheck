#pragma once

namespace rc {
namespace seq {
namespace detail {

template<typename T, int N>
class JustSeq
{
public:
    template<typename ...Args>
    JustSeq(Args &&...args)
        : m_values{std::forward<Args>(args)...}
        , m_next(0) {}

    bool hasNext() const { return m_next < N; }
    T next() { return std::move(m_values[m_next++]); }

private:
    std::array<T, N> m_values;
    std::size_t m_next;
};


template<typename Container>
class ContainerSeq
{
public:
    typedef Decay<typename Container::value_type> T;

    template<typename C,
             typename = typename std::enable_if<
                 std::is_constructible<Container, C>::value>::type>
    ContainerSeq(C &&container)
        : m_container(std::forward<C>(container))
        , m_iterator(begin(m_container)) {}

    ContainerSeq(const ContainerSeq &other)
        : m_container(other.m_container)
        , m_iterator(begin(m_container))
    {
        // TODO dat const_cast? safe?
        std::advance(m_iterator,
                     std::distance(
                         begin(const_cast<Container &>(other.m_container)),
                         other.m_iterator));
    }

    ContainerSeq &operator=(const ContainerSeq &other)
    {
        m_container = other.m_container;
        m_iterator = begin(m_container);
        std::advance(m_iterator,
                     std::distance(
                         begin(const_cast<Container &>(other.m_container)),
                         other.m_iterator));
        return *this;
    }

    ContainerSeq(ContainerSeq &&other) { moveFrom(other); }

    ContainerSeq &operator=(ContainerSeq &&other)
    {
        moveFrom(other);
        return *this;
    }

    bool hasNext() const { return m_iterator != end(m_container); }
    T next() { return std::move(*(m_iterator++)); }

private:
    void moveFrom(ContainerSeq &other)
    {
        const auto i = std::distance(
            begin(const_cast<Container &>(other.m_container)),
            other.m_iterator);
        m_container = std::move(other.m_container);
        m_iterator = begin(m_container);
        std::advance(m_iterator, i);
    }

    Container m_container;
    typename Container::iterator m_iterator;
};

} // namespace detail

template<typename T, typename ...Ts>
Seq<Decay<T>> just(T &&value, Ts &&...values)
{
    return detail::JustSeq<Decay<T>, sizeof...(Ts) + 1>{
        std::forward<T>(value), std::forward<Ts>(values)...};
}

template<typename Container>
Seq<Decay<typename Decay<Container>::value_type>>
fromContainer(Container &&container)
{
    typedef Decay<Container> ContainerT;
    typedef Decay<typename ContainerT::value_type> T;

    if (container.empty())
        return Seq<T>();

    return detail::ContainerSeq<ContainerT>(std::forward<Container>(container));
}

} // namespace seq
} // namespace rc
