#pragma once

#include "Numeric.h"

namespace rc {
namespace gen {

template<typename Container>
class ElementOf : public gen::Generator<typename Container::value_type>
{
public:
    typedef typename Container::value_type T;

    ElementOf(std::initializer_list<T> elements)
        : m_container(elements) {}

    template<typename ...Args>
    ElementOf(Args &&...args)
        : m_container(std::forward<Args>(args)...) {}

    T generate() const override
    {
        if (m_container.empty())
            throw GenerationFailure("Cannot choose value from empty container.");

        typedef typename Container::size_type SizeT;
        auto it = begin(m_container);
        const SizeT size = std::distance(it, end(m_container));
        const auto n = *noShrink(ranged<SizeT>(0, size));
        std::advance(it, n);
        return *it;
    }

private:
    Container m_container;
};

template<typename Container>
ElementOf<detail::DecayT<Container>> elementOf(Container &&container)
{
    return ElementOf<detail::DecayT<Container>>(
        std::forward<Container>(container));
}

template<typename Arg, typename ...Args>
ElementOf<std::vector<Arg>>
element(Arg &&arg, Args &&...args)
{
    return ElementOf<std::vector<Arg>>{
        std::forward<Arg>(arg), std::forward<Args>(args)...
    };
}

} // namespace gen
} // namespace rc
