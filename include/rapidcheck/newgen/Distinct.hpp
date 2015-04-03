#pragma once

namespace rc {
namespace newgen {
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
    return newgen::suchThat(
        std::move(gen),
        detail::DistinctPredicate<Decay<Value>>(std::forward<Value>(value)));
}

template<typename Value>
Gen<Decay<Value>> distinctFrom(Value &&value)
{
    return newgen::suchThat(
        newgen::arbitrary<Decay<Value>>(),
        detail::DistinctPredicate<Decay<Value>>(std::forward<Value>(value)));
}

} // namespace newgen
} // namespace rc
