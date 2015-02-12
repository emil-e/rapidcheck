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
    : public gen::PairOf<Arbitrary<Decay<T1>>, Arbitrary<Decay<T2>>>
{
public:
    Arbitrary()
        : gen::PairOf<Arbitrary<Decay<T1>>, Arbitrary<Decay<T2>>>(
            gen::arbitrary<Decay<T1>>(),
            gen::arbitrary<Decay<T2>>()) {}
};

} // namespace rc
