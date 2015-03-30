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
    typedef typename std::result_of<Mapper(T)>::type U;

    template<typename MapperArg>
    MapGen(MapperArg &&mapper, Gen<T> gen)
        : m_mapper(std::forward<MapperArg>(mapper))
        , m_gen(std::move(gen)) {}

    Shrinkable<U> operator()(const Random &random, int size) const
    { return shrinkable::map(m_mapper, m_gen(random, size)); }

private:
    Mapper m_mapper;
    Gen<T> m_gen;
};

template<typename T, typename Predicate>
class FilterGen
{
public:
    template<typename PredicateArg>
    FilterGen(PredicateArg &&predicate, Gen<T> gen)
        : m_predicate(std::forward<PredicateArg>(predicate))
        , m_gen(std::move(gen)) {}

    Shrinkable<T> operator()(const Random &random, int size) const
    {
        Random r(random);
        int currentSize = size;
        for (int tries = 0; tries < 100; tries++) {
            auto shrinkable = shrinkable::filter(
                m_predicate, m_gen(r.split(), currentSize));

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
Gen<typename std::result_of<Mapper(T)>::type> map(Mapper &&mapper, Gen<T> gen)
{
    return detail::MapGen<T, Decay<Mapper>>(std::forward<Mapper>(mapper),
                                            std::move(gen));
}

template<typename T, typename Mapper>
Gen<typename std::result_of<Mapper(T)>::type> map(Mapper &&mapper)
{ return newgen::map(std::forward<Mapper>(mapper), newgen::arbitrary<T>()); }

template<typename T, typename U>
Gen<T> cast(Gen<U> gen)
{
    return newgen::map(
        [](U &&x) { return static_cast<T>(std::move(x)); },
        std::move(gen));
}

template<typename T, typename Predicate>
Gen<T> suchThat(Predicate &&pred, Gen<T> gen)
{
    return detail::FilterGen<T, Decay<Predicate>>(
        std::forward<Predicate>(pred), std::move(gen));
}

template<typename T, typename Predicate>
Gen<T> suchThat(Predicate &&pred)
{
    return newgen::suchThat(std::forward<Predicate>(pred),
                            newgen::arbitrary<T>());
}

} // namespace newgen
} // namespace rc
