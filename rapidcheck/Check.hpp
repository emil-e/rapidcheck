#pragma once

#include "Generators.hpp"

namespace rc {

template<typename Generator>
GeneratedType<Generator> pick(const Generator &gen)
{
    return gen(50);
}

template<typename T>
T pick()
{
    return pick(arbitrary<T>());
}

template<typename T> class Property;

template<typename Functor, typename ...Args>
class Property<bool (Functor::*)(Args...) const>
{
public:
    Property(const Functor &functor) : m_functor(functor) {}

    bool operator()() const
    { return m_functor(pick<typename std::decay<Args>::type>()...); }

private:
    Functor m_functor;
};

template<typename Testable>
bool check(Testable prop)
{
    return Property<decltype(&Testable::operator())>(prop)();
}

}
