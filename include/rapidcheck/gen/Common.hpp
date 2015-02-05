#pragma once

#include "Generator.h"

namespace rc {
namespace gen {

template<typename Gen, typename Predicate>
class SuchThat : public Generator<GeneratedT<Gen>>
{
public:
    SuchThat(Gen generator, Predicate predicate)
        : m_generator(std::move(generator))
        , m_predicate(std::move(predicate)) {}

    GeneratedT<Gen> generate() const override
    {
        auto startSize = currentSize();
        auto size = startSize;
        while (true) { // TODO give up sometime
            auto x(*noShrink(resize(size, m_generator)));
            if (m_predicate(x))
                return x;
            size++;
            if ((size - startSize) > 100) {
                throw GenerationFailure(
                    "Gave up trying to generate value satisfying predicate");
            }
        }
    }

private:
    Gen m_generator;
    Predicate m_predicate;
};

template<typename Gen, typename Mapper>
class Mapped : public Generator<
    typename std::result_of<Mapper(GeneratedT<Gen>)>::type>
{
public:
    typedef typename
    std::result_of<Mapper(GeneratedT<Gen>)>::type T;

    Mapped(Gen generator, Mapper mapper)
        : m_generator(std::move(generator))
        , m_mapper(std::move(mapper)) {}

    T generate() const override { return m_mapper(*m_generator); }

private:
    Gen m_generator;
    Mapper m_mapper;
};

template<typename T>
class Constant : public Generator<T>
{
public:
    explicit Constant(T value) : m_value(std::move(value)) {}
    T generate() const override { return m_value; }

private:
    T m_value;
};

template<typename Generator, typename Predicate>
SuchThat<Generator, Predicate> suchThat(Generator gen,
                                        Predicate pred)
{ return SuchThat<Generator, Predicate>(std::move(gen), std::move(pred)); }

template<typename T, typename Predicate>
SuchThat<Arbitrary<T>, Predicate> suchThat(Predicate pred)
{ return suchThat(arbitrary<T>(), std::move(pred)); }

template<typename Gen, typename Mapper>
Mapped<Gen, Mapper> map(Gen generator, Mapper mapper)
{ return Mapped<Gen, Mapper>(std::move(generator), std::move(mapper)); }

template<typename T>
Constant<T> constant(T value) { return Constant<T>(std::move(value)); }

} // namespace gen
} // namespace rc
