#pragma once

#include "rapidcheck/shrinkable/Create.h"
#include "rapidcheck/seq/Transform.h"

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

template<typename T, typename Predicate>
Maybe<Shrinkable<T>> filter(Predicate pred, Shrinkable<T> shrinkable)
{
    if (!pred(shrinkable.value()))
        return Nothing;

    return shrinkable::mapShrinks([=](Seq<Shrinkable<T>> &&shrinks) {
        return seq::mapMaybe([=](Shrinkable<T> &&shrink) {
            return shrinkable::filter(pred, std::move(shrink));
        }, std::move(shrinks));
    }, std::move(shrinkable));
}

template<typename T1, typename T2>
Shrinkable<std::pair<T1, T2>> pair(Shrinkable<T1> s1, Shrinkable<T2> s2)
{
    return shrinkable::map(
        [](const std::pair<Shrinkable<T1>, Shrinkable<T2>> &p) {
            return std::make_pair(p.first.value(), p.second.value());
        },
        shrinkable::shrinkRecur(
            std::make_pair(s1, s2),
            [](const std::pair<Shrinkable<T1>, Shrinkable<T2>> &p) {
                return seq::concat(
                    seq::map([=](Shrinkable<T1> &&s) {
                        return std::make_pair(std::move(s), p.second);
                    }, p.first.shrinks()),
                    seq::map([=](Shrinkable<T2> &&s) {
                        return std::make_pair(p.first, std::move(s));
                    }, p.second.shrinks()));
            }));
}

} // namespace shrinkable
} // namespace rc
