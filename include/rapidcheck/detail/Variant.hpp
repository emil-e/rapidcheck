#pragma once

#include <type_traits>

namespace rc {
namespace detail {

template<typename ...Types>
struct IndexHelper;

template<>
struct IndexHelper<>
{
    template<typename T>
    static constexpr std::ptrdiff_t indexOf() { return 0; }
};

template<typename First, typename ...Types>
struct IndexHelper<First, Types...>
{
    template<typename T>
    static constexpr std::ptrdiff_t indexOf()
    {
        return std::is_same<First, T>::value
            ? 0
            : IndexHelper<Types...>::template indexOf<T>() + 1;
    }
};

template<typename ...Types>
template<typename T>
Variant<Types...>::Variant(T &&value)
    : m_typeIndex(indexOfType<DecayT<T>>())
    , m_value(Any::of(std::forward<T>(value)))
{
    static_assert(isValidType<DecayT<T>>(),
                  "T is not a valid type of this variant");
}

// TODO this would be more fun with varargs and lambdas
template<typename ...Types>
template<typename T>
bool Variant<Types...>::match(T &value) const
{
    if (!is<T>())
        return false;

    value = m_value.get<T>();
    return true;
}

template<typename ...Types>
template<typename T>
bool Variant<Types...>::is() const
{
    static_assert(isValidType<DecayT<T>>(),
                  "T is not a valid type of this variant");
    return m_typeIndex == indexOfType<T>();
}

template<typename ...Types>
template<typename T>
constexpr std::ptrdiff_t Variant<Types...>::indexOfType()
{ return IndexHelper<Types...>::template indexOf<T>(); }

template<typename ...Types>
template<typename T>
constexpr bool Variant<Types...>::isValidType()
{ return indexOfType<T>() < sizeof...(Types); }

template<typename T>
bool equalAs(const Any &lhs, const Any &rhs)
{ return lhs.get<T>() == rhs.get<T>(); }

template<typename ...Types>
bool operator==(const Variant<Types...> &v1, const Variant<Types...> &v2)
{
    typedef bool (*EqualFunc)(const Any &, const Any &);
    static EqualFunc equalFuncs[sizeof...(Types)] = { equalAs<Types>... };

    return
        (v1.m_typeIndex == v2.m_typeIndex) &&
        equalFuncs[v1.m_typeIndex](v1.m_value, v2.m_value);
}

template<typename ...Types>
bool operator!=(const Variant<Types...> &v1, const Variant<Types...> &v2)
{ return !(v1 == v2); }

template<typename ...Types>
std::ostream &operator<<(std::ostream &os, const Variant<Types...> &variant)
{
    const auto typeValue = variant.m_value.describe();
    os << typeValue.first << ": " << typeValue.second;
    return os;
}

} // namespace detail
} // namespace rc
