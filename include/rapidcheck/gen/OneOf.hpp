#pragma once

#include "Generator.h"
#include "Parameters.h"

namespace rc {
namespace gen {

// Helper class for OneOf to be able to have a collection of generators of
// different types
template<typename ...Gens>
class Multiplexer;

template<typename Gen, typename ...Gens>
class Multiplexer<Gen, Gens...>
{
public:
    typedef GeneratedT<Gen> GeneratedType;
    static constexpr int numGenerators = sizeof...(Gens) + 1;

    static_assert(
        std::is_same<
        GeneratedT<Gen>,
            typename std::tuple_element<0,
            std::tuple<GeneratedT<Gens>...>>::type>::value,
        "All generators must have the same result type");

    Multiplexer(Gen generator, Gens... generators)
        : m_generator(std::move(generator))
        , m_multiplexer(std::move(generators)...) {}

    GeneratedT<Gen> pickWithId(int id) const
    {
        if (id == myId)
            return *m_generator;
        else
            return m_multiplexer.pickWithId(id);
    }

private:
    static constexpr int myId = sizeof...(Gens);

    Gen m_generator;
    Multiplexer<Gens...> m_multiplexer;
};

template<typename Gen>
class Multiplexer<Gen>
{
public:
    typedef GeneratedT<Gen> GeneratedType;
    static constexpr int numGenerators = 1;

    Multiplexer(Gen generator)
        : m_generator(std::move(generator)) {}

    GeneratedT<Gen> pickWithId(int id) const
    { return *m_generator; }

private:
    static constexpr int myId = 0;

    Gen m_generator;
};

template<typename ...Gens>
class OneOf : public Generator<GeneratedT<Multiplexer<Gens...>>>
{
public:
    OneOf(Gens... generators) : m_multiplexer(std::move(generators)...) {}

    GeneratedT<Multiplexer<Gens...>> generate() const override
    {
        int n = Multiplexer<Gens...>::numGenerators;
        auto id = *resize(kNominalSize, ranged<int>(0, n));
        return m_multiplexer.pickWithId(id);
    }

private:
    Multiplexer<Gens...> m_multiplexer;
};

template<typename ...Gens>
OneOf<Gens...> oneOf(Gens... generators)
{ return OneOf<Gens...>(std::move(generators)...); }

} // namespace gen
} // namespace rc
