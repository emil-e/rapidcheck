#pragma once

#include <type_traits>

#include "Variant.h"
#include "Traits.h"

namespace rc {
namespace detail {

template<typename ...Types>
struct IndexHelper;

template<>
struct IndexHelper<>
{
    template<typename T>
    static constexpr int indexOf() { return -1; }
};

template<typename First, typename ...Types>
struct IndexHelper<First, Types...>
{
    template<typename T>
    static constexpr int indexOf()
    {
        return std::is_same<First, T>::value
            ? sizeof...(Types)
            : IndexHelper<Types...>::template indexOf<T>();
    }
};

template<typename ...Types>
template<typename T>
Variant<Types...>::Variant(T &&value)
    : m_copy([](void *v) -> void * {
        return new DecayT<T>(*static_cast<DecayT<T> *>(v)); })
    , m_delete([](void *v){ delete static_cast<DecayT<T> *>(v); })
    , m_typeIndex(indexOfType<T>())
    , m_value(new DecayT<T>(std::forward<T>(value)))
{
    static_assert(indexOfType<DecayT<T>>() != -1,
                  "T is not a valid type of this variant");
}

template<typename ...Types>
Variant<Types...>::Variant(const Variant<Types...> &other)
    : m_copy(other.m_copy)
    , m_delete(other.m_delete)
    , m_typeIndex(other.m_typeIndex)
    , m_value(other.m_copy(other.m_value)) {}

template<typename ...Types>
Variant<Types...>::Variant(Variant<Types...> &&other)
    : m_copy(other.m_copy)
    , m_delete(other.m_delete)
    , m_typeIndex(other.m_typeIndex)
    , m_value(other.m_value)
{ other.m_value = nullptr; }

template<typename ...Types>
Variant<Types...> &Variant<Types...>::operator=(const Variant<Types...> &rhs)
{
    m_delete(m_value);
    m_copy = rhs.m_copy;
    m_delete = rhs.m_delete;
    m_typeIndex = rhs.m_typeIndex;
    m_value = m_copy(rhs.m_value);
}

template<typename ...Types>
Variant<Types...> &Variant<Types...>::operator=(Variant<Types...> &&rhs)
{
    m_delete(m_value);
    m_copy = rhs.m_copy;
    m_delete = rhs.m_delete;
    m_typeIndex = rhs.m_typeIndex;
    m_value = rhs.m_value;
    rhs.m_value = nullptr;
}

template<typename ...Types>
template<typename T>
bool Variant<Types...>::match(T &value) const
{
    static_assert(indexOfType<T>() != -1,
                  "T is not a valid type of this variant");

    if (m_typeIndex != indexOfType<T>())
        return false;

    value = *static_cast<T *>(m_value);
    return true;
}

template<typename ...Types>
template<typename T>
bool Variant<Types...>::is() const
{
    static_assert(indexOfType<T>() != -1,
                  "T is not a valid type of this variant");
    return m_typeIndex == indexOfType<T>();
}

template<typename ...Types>
Variant<Types...>::~Variant() { m_delete(m_value); }

template<typename ...Types>
template<typename T>
constexpr int Variant<Types...>::indexOfType()
{
    return IndexHelper<Types...>::template indexOf<T>();
}

} // namespace detail
} // namespace rc
