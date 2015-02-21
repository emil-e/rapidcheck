#pragma once

#include "Generator.h"

namespace rc {
namespace gen {

template<typename ...Gens>
class TupleOf;

template<>
class TupleOf<> : public Generator<std::tuple<>>
{
public:
    std::tuple<> generate() const override { return std::tuple<>(); }
};

template<typename Gen, typename ...Gens>
class TupleOf<Gen, Gens...>
    : public Generator<std::tuple<GeneratedT<Gen>,
                                  GeneratedT<Gens>...>>
{
public:
    typedef std::tuple<GeneratedT<Gen>,
                       GeneratedT<Gens>...> TupleT;
    typedef GeneratedT<Gen> HeadT;
    typedef std::tuple<GeneratedT<Gens>...> TailT;

    TupleOf(Gen headGenerator, Gens ...tailGenerators)
        : m_headGenerator(std::move(headGenerator))
        , m_tailGenerator(std::move(tailGenerators)...) {}

    TupleT generate() const override
    {
        return std::tuple_cat(
            std::tuple<GeneratedT<Gen>>(*m_headGenerator),
            *m_tailGenerator);
    }

    Seq<TupleT> shrink(TupleT value) const override
    { return shrink(value, detail::IsCopyConstructible<TupleT>()); }

private:
    Seq<TupleT> shrink(const TupleT &value, std::false_type) const
    { return Seq<TupleT>(); }

    Seq<TupleT> shrink(const TupleT &value, std::true_type) const
    {
        // Shrink the head and map it by append the unshrunk tail,
        // then shrink the tail and map it by prepending the unshrink head.
        return seq::concat(
            seq::map([=] (HeadT &&x) -> TupleT {
                         return std::tuple_cat(
                             std::tuple<HeadT>(std::move(x)),
                             detail::tupleTail(value));
            }, m_headGenerator.shrink(std::get<0>(value))),
            seq::map([=] (TailT &&x) -> TupleT {
                         return std::tuple_cat(
                             std::tuple<HeadT>(std::get<0>(value)),
                             std::move(x));
            }, m_tailGenerator.shrink(detail::tupleTail(value))));
    }

    Gen m_headGenerator;
    TupleOf<Gens...> m_tailGenerator;
};

template<typename Gen1, typename Gen2>
class PairOf : public Generator<std::pair<GeneratedT<Gen1>,
                                          GeneratedT<Gen2>>>
{
public:
    typedef GeneratedT<Gen1> T1;
    typedef GeneratedT<Gen2> T2;
    typedef typename std::pair<T1, T2> PairT;

    PairOf(Gen1 generator1, Gen2 generator2)
        : m_generator(std::move(generator1),
                      std::move(generator2)) {}

    PairT generate() const override
    {
        auto x = m_generator.generate();
        return PairT(std::move(std::get<0>(x)),
                     std::move(std::get<1>(x)));
    }

    Seq<PairT> shrink(PairT value) const override
    {
        return seq::map(
            [] (std::tuple<T1, T2> &&x) {
                return PairT(std::move(std::get<0>(x)),
                             std::move(std::get<1>(x)));
            },
            m_generator.shrink(std::tuple<T1, T2>(std::move(value.first),
                                                  std::move(value.second))));
    }

private:
    TupleOf<Gen1, Gen2> m_generator;
};

template<typename ...Gens>
TupleOf<Gens...> tupleOf(Gens ...generators)
{ return TupleOf<Gens...>(std::move(generators)...); }

template<typename Gen1, typename Gen2>
PairOf<Gen1, Gen2> pairOf(Gen1 generator1, Gen2 generator2)
{ return PairOf<Gen1, Gen2>(std::move(generator1), std::move(generator2)); }

} // namespace gen
} // namespace rc
