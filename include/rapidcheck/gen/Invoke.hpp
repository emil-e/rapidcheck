#pragma once

#include "Generator.h"

namespace rc {
namespace gen {

template<typename Callable>
class AnyInvocation : public Generator<
    typename detail::Quantifier<Callable>::ReturnType>
{
public:
    explicit AnyInvocation(Callable callable)
        : m_quantifier(std::move(callable)) {}

    typename detail::Quantifier<Callable>::ReturnType
    generate() const override
    { return m_quantifier(); }

private:
    detail::Quantifier<Callable> m_quantifier;
};


template<typename Exception, typename Gen, typename Catcher>
class Rescue : public Generator<GeneratedT<Gen>>
{
public:
    Rescue(Gen generator, Catcher catcher)
        : m_generator(generator), m_catcher(catcher) {}

    GeneratedT<Gen> generate() const override
    {
        try {
            return m_generator.generate();
        } catch (const Exception &e) {
            return m_catcher(e);
        }
    }

private:
    Gen m_generator;
    Catcher m_catcher;
};

template<typename Callable>
class Lambda : public Generator<typename std::result_of<Callable()>::type>
{
public:
    explicit Lambda(Callable callable) : m_callable(std::move(callable)) {}

    typename std::result_of<Callable()>::type
    generate() const override { return m_callable(); }

private:
    Callable m_callable;
};

template<typename Callable>
AnyInvocation<Callable> anyInvocation(Callable callable)
{ return AnyInvocation<Callable>(std::move(callable)); }

// TODO deduce exception type
template<typename Exception, typename Gen, typename Catcher>
Rescue<Exception, Gen, Catcher> rescue(Gen generator, Catcher catcher)
{
    return Rescue<Exception, Gen, Catcher>(std::move(generator),
                                           std::move(catcher));
}

template<typename Callable>
Lambda<Callable> lambda(Callable callable)
{ return Lambda<Callable>(std::move(callable)); }

} // namespace gen
} // namespace rc
