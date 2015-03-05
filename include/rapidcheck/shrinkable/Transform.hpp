#pragma once

namespace rc {
namespace shrinkable {
namespace detail {

template<typename T, typename Mapper>
class MapShrinkable
{
public:
    typedef typename std::result_of<Mapper(T)>::type U;

    template<typename MapperArg>
    MapShrinkable(MapperArg &&mapper, Shrinkable<T> shrinkable)
        : m_mapper(std::forward<MapperArg>(mapper))
        , m_shrinkable(std::move(shrinkable)) {}

    U value() const { return m_mapper(m_shrinkable.value()); }

    Seq<Shrinkable<U>> shrinks() const
    {
        auto mapper = m_mapper;
        return seq::map([=](Shrinkable<T> &&shrink) {
            return shrinkable::map(mapper, shrink);
        }, m_shrinkable.shrinks());
    }

private:
    Mapper m_mapper;
    Shrinkable<T> m_shrinkable;
};

template<typename T, typename Mapper>
class MapShrinksShrinkable
{
public:
    template<typename MapperArg>
    MapShrinksShrinkable(MapperArg &&mapper, Shrinkable<T> shrinkable)
        : m_mapper(std::forward<MapperArg>(mapper))
        , m_shrinkable(std::move(shrinkable)) {}

    T value() const { return m_shrinkable.value(); }

    Seq<Shrinkable<T>> shrinks() const
    { return m_mapper(m_shrinkable.shrinks()); }

private:
    Mapper m_mapper;
    Shrinkable<T> m_shrinkable;
};

} // namespace detail

template<typename T, typename Mapper>
Shrinkable<typename std::result_of<Mapper(T)>::type>
map(Mapper &&mapper, Shrinkable<T> shrinkable)
{
    typedef detail::MapShrinkable<T, Decay<Mapper>> Impl;
    return makeShrinkable<Impl>(std::forward<Mapper>(mapper),
                                std::move(shrinkable));
}

template<typename T, typename Mapper>
Shrinkable<T> mapShrinks(Mapper &&mapper, Shrinkable<T> shrinkable)
{
    typedef detail::MapShrinksShrinkable<T, Decay<Mapper>> Impl;
    return makeShrinkable<Impl>(std::forward<Mapper>(mapper),
                                std::move(shrinkable));
}

} // namespace shrinkable
} // namespace rc
