#pragma once

namespace rc {
namespace predicate {

template<typename Predicate>
template<typename T>
Not<Predicate>::Not(T &&predicate)
    : m_predicate(std::forward<T>(predicate)) {}

template<typename Predicate>
template<typename T>
bool Not<Predicate>::operator()(const T &arg) const
{ return !m_predicate(arg); }

#define DEFINE_BINARY_PREDICATE(Name, expr)             \
    template<typename T>                                \
    template<typename U>                                \
    Name<T>::Name(U &&value)                            \
        : m_value(std::forward<U>(value)) {}            \
                                                        \
    template<typename T>                                \
    template<typename U>                                \
    bool Name<T>::operator()(const U &arg) const        \
    { return (expr); }                                  \

DEFINE_BINARY_PREDICATE(Equals, arg == m_value)
DEFINE_BINARY_PREDICATE(GreaterThan, arg > m_value)
DEFINE_BINARY_PREDICATE(LessThan, arg < m_value)
DEFINE_BINARY_PREDICATE(GreaterEqThan, arg >= m_value)
DEFINE_BINARY_PREDICATE(LessEqThan, arg <= m_value)

#undef DEFINE_BINARY_PREDICATE

} // namespace predicate
} // namespace rc
