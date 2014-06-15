#pragma once

#include "Variant.h"

namespace rc {
namespace detail {

template<typename ...Args>
struct IndexHelper;

template<>
struct IndexHelper<>
{
    template<typename T>
    static constexpr int indexOf() { return -1; }
};

template<typename First, typename ...Args>
struct IndexHelper<First, Args...>
{
    template<typename T>
    static constexpr int indexOf()
    {
        return std::is_same<First, T>::value
            ? 0
            : IndexHelper<Args...>::template indexOf<T>() + 1;
    }
};

template<typename ...Args>
template<typename T>
Variant<Args...>::Variant(T value)
    : m_copy([](void *v) -> void * { return new T(*static_cast<T *>(v)); })
    , m_delete([](void *v){ delete static_cast<T *>(v); })
    , m_typeIndex(indexOfType<T>())
    , m_value(new T(std::move(value)))
{ static_assert(indexOfType<T>() != -1, "T is not a valid type of this variant"); }

template<typename ...Args>
Variant<Args...>::Variant(const Variant<Args...> &other)
    : m_copy(other.m_copy)
    , m_delete(other.m_delete)
    , m_typeIndex(other.m_typeIndex)
    , m_value(m_copy(other.m_value)) {}

template<typename ...Args>
Variant<Args...>::Variant(Variant<Args...> &&other)
    : m_copy(other.m_copy)
    , m_delete(other.m_delete)
    , m_typeIndex(other.m_typeIndex)
    , m_value(other.m_value)
{ other.m_value = nullptr; }

template<typename ...Args>
Variant<Args...> &Variant<Args...>::operator=(const Variant<Args...> &rhs)
{
    m_copy = rhs.m_copy;
    m_delete = rhs.m_delete;
    m_typeIndex = rhs.m_typeIndex;
    m_value = m_copy(rhs.m_value);
}

template<typename ...Args>
Variant<Args...> &Variant<Args...>::operator=(Variant<Args...> &&rhs)
{
    m_copy = rhs.m_copy;
    m_delete = rhs.m_delete;
    m_typeIndex = rhs.m_typeIndex;
    m_value = rhs.m_value;
    rhs.m_value = nullptr;
}

template<typename ...Args>
template<typename T>
bool Variant<Args...>::match(T &value) const
{
    static_assert(indexOfType<T>() != -1,
                  "T is not a valid type of this variant");

    if (m_typeIndex != indexOfType<T>())
        return false;

    value = *static_cast<T *>(m_value);
    return true;
}

template<typename ...Args>
template<typename T>
bool Variant<Args...>::is() const
{
    static_assert(indexOfType<T>() != -1,
                  "T is not a valid type of this variant");
    return m_typeIndex == indexOfType<T>();
}

template<typename ...Args>
Variant<Args...>::~Variant() { m_delete(m_value); }

template<typename ...Args>
template<typename T>
constexpr int Variant<Args...>::indexOfType()
{
    return IndexHelper<Args...>::template indexOf<T>();
}

} // namespace detail
} // namespace rc
