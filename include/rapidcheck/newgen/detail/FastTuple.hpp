#pragma once

#include "rapidcheck/shrinkable/Create.h"
#include "rapidcheck/newgen/Arbitrary.h"
#include "rapidcheck/newgen/Transform.h"

namespace rc {
namespace newgen {
namespace detail {

template<typename ...Ts>
using FastTuple = rc::detail::FastTuple<Ts...>;

template<typename Indexes, typename ...Ts>
class FastTupleGen;

template<typename ...Ts,
         std::size_t ...Indexes>
class FastTupleGen<rc::detail::IndexSequence<Indexes...>, Ts...>
{
public:

    template<typename ...Args>
    explicit FastTupleGen(Args &&...args)
        : m_gens(std::forward<Args>(args)...) {}

    Shrinkable<FastTuple<Ts...>> operator()(
        const Random &random, int size) const
    {
        Random r(random);
        Random randoms[sizeof...(Ts)];
        for (std::size_t i = 0; i < sizeof...(Ts); i++)
            randoms[i] = r.split();

        return joinTuple((m_gens.template at<Indexes>()(randoms[Indexes], size))...);
    }

private:
    static Shrinkable<FastTuple<Ts...>> joinTuple(
        Shrinkable<Ts> &&...shrinkables)
    {
        return shrinkable::map(
            shrinkable::shrinkRecur(
                FastTuple<Shrinkable<Ts>...>(std::move(shrinkables)...),
                &FastTupleGen::shrinkTuple),
            [](FastTuple<Shrinkable<Ts>...> &&st) {
                return FastTuple<Ts...>((st.template at<Indexes>().value())...);
            });
    }

    static Seq<FastTuple<Shrinkable<Ts>...>> shrinkTuple(
        FastTuple<Shrinkable<Ts>...> &&tuple)
    {
        return seq::concat(shrinkComponent<Indexes>(tuple)...);
    }

    template<std::size_t N>
    static Seq<FastTuple<Shrinkable<Ts>...>> shrinkComponent(
        const FastTuple<Shrinkable<Ts>...> &tuple)
    {
        using ShrinkableT =
            typename decltype(tuple.template at<N>().shrinks())::ValueType;
        return seq::map(
            tuple.template at<N>().shrinks(),
            [=](ShrinkableT &&cshrink) {
                auto shrink(tuple);
                shrink.template at<N>() = cshrink;
                return shrink;
            });
    }

    FastTuple<Gen<Ts>...> m_gens;
};

// Specialization for empty tuples.
template<>
class FastTupleGen<rc::detail::IndexSequence<>>
{
public:
    Shrinkable<FastTuple<>> operator()(const Random &random, int size) const
    { return shrinkable::just(FastTuple<>()); }
};

template<typename ...Ts>
struct DefaultArbitrary<FastTuple<Ts...>>
{
    static Gen<FastTuple<Decay<Ts>...>> arbitrary()
    {
        return fastTuple(newgen::arbitrary<Decay<Ts>>()...);
    }
};

template<typename ...Ts>
Gen<FastTuple<Ts...>> fastTuple(Gen<Ts> ...gens)
{
    return detail::FastTupleGen<rc::detail::MakeIndexSequence<sizeof...(Ts)>, Ts...>(
        std::move(gens)...);
}

} // namespace detail
} // namespace newgen
} // namespace rc
