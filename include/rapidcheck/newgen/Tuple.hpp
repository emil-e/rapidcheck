#pragma once

#include "rapidcheck/shrinkable/Create.h"

namespace rc {
namespace newgen {
namespace detail {

template<typename Indexes, typename ...Ts>
class TupleGen;

template<typename ...Ts,
         std::size_t ...Indexes>
class TupleGen<rc::detail::IndexSequence<Indexes...>, Ts...>
{
public:
    template<typename ...Args>
    explicit TupleGen(Args &&...args)
        : m_gens(std::forward<Args>(args)...) {}

    Shrinkable<std::tuple<Ts...>> operator()(
        const Random &random, int size) const
    {
        Random r(random);
        Random randoms[sizeof...(Ts)];
        for (std::size_t i = 0; i < sizeof...(Ts); i++)
            randoms[i] = r.split();

        return joinTuple(
            std::make_tuple(std::get<Indexes>(m_gens)(randoms[Indexes], size)...));
    }

private:
    static Shrinkable<std::tuple<Ts...>> joinTuple(
        std::tuple<Shrinkable<Ts>...> &&tuple)
    {
        return shrinkable::map([](std::tuple<Shrinkable<Ts>...> &&st) {
            return std::make_tuple(std::get<Indexes>(st).value()...);
        }, shrinkable::shrinkRecur(std::move(tuple), &TupleGen::shrinkTuple));
    }

    static Seq<std::tuple<Shrinkable<Ts>...>> shrinkTuple(
        std::tuple<Shrinkable<Ts>...> &&tuple)
    {
        return seq::concat(shrinkComponent<Indexes>(tuple)...);
    }

    template<std::size_t N>
    static Seq<std::tuple<Shrinkable<Ts>...>> shrinkComponent(
        const std::tuple<Shrinkable<Ts>...> &tuple)
    {
        typedef typename std::tuple_element<N, std::tuple<Ts...>>::type T;
        return seq::map([=](Shrinkable<T> &&cshrink) {
            auto shrink(tuple);
            std::get<N>(shrink) = cshrink;
            return shrink;
        }, std::get<N>(tuple).shrinks());
    }

    std::tuple<Gen<Ts>...> m_gens;
};


} // namespace detail

template<typename ...Ts>
Gen<std::tuple<Ts...>> tuple(Gen<Ts> ...gens)
{
    return detail::TupleGen<rc::detail::MakeIndexSequence<sizeof...(Ts)>, Ts...>(
        std::move(gens)...);
}

} // namespace newgen
} // namespace rc
