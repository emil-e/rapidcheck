#pragma once

namespace rc {
namespace newgen {
namespace detail {

template<typename Container>
class ElementOfGen
{
public:
    typedef typename Container::value_type T;

    template<typename Arg>
    ElementOfGen(Arg &&arg)
        : m_container(std::forward<Arg>(arg)) {}

    Shrinkable<T> operator()(const Random &random, int size) const
    {
        const auto start = begin(m_container);
        const auto containerSize = end(m_container) - start;
        if (containerSize == 0) {
            throw GenerationFailure(
                "Cannot pick element from empty container.");
        }
        const auto i = Random(random).next() % containerSize;
        return shrinkable::just(*(start + i));
    }

private:
    Container m_container;
};

} // namespace detail

template<typename Container>
Gen<typename Decay<Container>::value_type> elementOf(Container &&container)
{
    return detail::ElementOfGen<Decay<Container>>(
        std::forward<Container>(container));
}

template<typename T, typename ...Ts>
Gen<Decay<T>> element(T &&element, Ts &&...elements)
{
    using Vector = std::vector<Decay<T>>;
    return detail::ElementOfGen<Vector>(Vector{
            std::forward<T>(element), std::forward<Ts>(elements)...});
}

} // namespace newgen
} // namespace rc
