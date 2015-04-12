#pragma once

namespace rc {
namespace gen {
namespace detail {

template<typename Value>
class DistinctPredicate
{
public:
    template<typename ValueArg,
             typename = typename std::enable_if<
                 !std::is_same<Decay<ValueArg>, DistinctPredicate>::value>::type>
    DistinctPredicate(ValueArg &&value)
        : m_value(std::forward<ValueArg>(value)) {}

    template<typename T>
    bool operator()(const T &other) const { return !(other == m_value); }

private:
    Value m_value;
};

} // namespace detail

template<typename T, typename Value>
Gen<T> distinctFrom(Gen<T> gen, Value &&value)
{
    return gen::suchThat(
        std::move(gen),
        detail::DistinctPredicate<Decay<Value>>(std::forward<Value>(value)));
}

template<typename Value>
Gen<Decay<Value>> distinctFrom(Value &&value)
{
    return gen::suchThat(
        gen::arbitrary<Decay<Value>>(),
        detail::DistinctPredicate<Decay<Value>>(std::forward<Value>(value)));
}

} // namespace gen
} // namespace rc
