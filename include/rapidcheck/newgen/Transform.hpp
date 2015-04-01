#pragma once

#include "rapidcheck/shrinkable/Transform.h"
#include "rapidcheck/newgen/Arbitrary.h"

namespace rc {
namespace newgen {
namespace detail {

template<typename T, typename Mapper>
class MapGen
{
public:
    typedef Decay<typename std::result_of<Mapper(T)>::type> U;

    template<typename MapperArg>
    MapGen(Gen<T> gen, MapperArg &&mapper)
        : m_mapper(std::forward<MapperArg>(mapper))
        , m_gen(std::move(gen)) {}

    Shrinkable<U> operator()(const Random &random, int size) const
    { return shrinkable::map(m_gen(random, size), m_mapper); }

private:
    Mapper m_mapper;
    Gen<T> m_gen;
};

template<typename T, typename Predicate>
class FilterGen
{
public:
    template<typename PredicateArg>
    FilterGen(Gen<T> gen, PredicateArg &&predicate)
        : m_predicate(std::forward<PredicateArg>(predicate))
        , m_gen(std::move(gen)) {}

    Shrinkable<T> operator()(const Random &random, int size) const
    {
        Random r(random);
        int currentSize = size;
        for (int tries = 0; tries < 100; tries++) {
            auto shrinkable = shrinkable::filter(
                m_gen(r.split(), currentSize), m_predicate);

            if (shrinkable)
                return std::move(*shrinkable);
            currentSize++;
        }

        throw GenerationFailure(
            "Gave up trying to generate value satisfying predicate.");
    }

private:
    Predicate m_predicate;
    Gen<T> m_gen;
};

} // namespace detail

template<typename T, typename Mapper>
Gen<Decay<typename std::result_of<Mapper(T)>::type>>
map(Gen<T> gen, Mapper &&mapper)
{
    return detail::MapGen<T, Decay<Mapper>>(std::move(gen),
                                            std::forward<Mapper>(mapper));
}

template<typename T, typename Mapper>
Gen<Decay<typename std::result_of<Mapper(T)>::type>> map(Mapper &&mapper)
{ return newgen::map(newgen::arbitrary<T>(), std::forward<Mapper>(mapper)); }

template<typename T, typename U>
Gen<T> cast(Gen<U> gen)
{
    return newgen::map(std::move(gen), [](U &&x) {
        return static_cast<T>(std::move(x));
    });
}

template<typename T, typename Predicate>
Gen<T> suchThat(Gen<T> gen, Predicate &&pred)
{
    return detail::FilterGen<T, Decay<Predicate>>(
        std::move(gen), std::forward<Predicate>(pred));
}

template<typename T, typename Predicate>
Gen<T> suchThat(Predicate &&pred)
{
    return newgen::suchThat(newgen::arbitrary<T>(),
                            std::forward<Predicate>(pred));
}

template<typename T>
Gen<T> resize(int size, Gen<T> gen)
{
    return [=](const Random &random, int) {
        return gen(random, size);
    };
}

template<typename T>
Gen<T> scale(double scale, Gen<T> gen)
{
    return [=](const Random &random, int size) {
        return gen(random, size * scale);
    };
}

} // namespace newgen
} // namespace rc
