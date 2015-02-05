#pragma once

namespace rc {

template<typename ...Types>
class Arbitrary<std::tuple<Types...>>
    : public gen::TupleOf<Arbitrary<Types>...>
{
public:
    Arbitrary() : gen::TupleOf<Arbitrary<Types>...>(
            gen::arbitrary<Types>()...) {}
};

template<typename T1, typename T2>
class Arbitrary<std::pair<T1, T2>>
    : public gen::PairOf<Arbitrary<detail::DecayT<T1>>,
                         Arbitrary<detail::DecayT<T2>>>
{
public:
    Arbitrary()
        : gen::PairOf<Arbitrary<detail::DecayT<T1>>,
                      Arbitrary<detail::DecayT<T2>>>(
            gen::arbitrary<detail::DecayT<T1>>(),
            gen::arbitrary<detail::DecayT<T2>>()) {}
};

} // namespace rc
