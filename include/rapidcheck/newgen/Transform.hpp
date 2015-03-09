#pragma once

#include "rapidcheck/shrinkable/Transform.h"

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

    Shrinkable<U> operator()(const Random &random, int size)
    { return shrinkable::map(m_mapper, m_gen(random, size)); }

private:
    Mapper m_mapper;
    Gen<T> m_gen;
};

} // namespace detail

template<typename T, typename Mapper>
Gen<typename std::result_of<Mapper(T)>::type> map(Mapper &&mapper, Gen<T> gen)
{
    return detail::MapGen<T, Decay<Mapper>>(std::forward<Mapper>(mapper),
                                            std::move(gen));
}

} // namespace newgen
} // namespace rc
