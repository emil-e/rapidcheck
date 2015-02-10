#pragma once

namespace rc {
namespace detail {

template<typename T>
ExpressionCaptor ExpressionCaptor::operator->*(const T &rhs) const
{ return appendShow(rhs); }

#define DEFINE_BINARY_OPERATOR(op)                                      \
    template<typename T>                                                \
    ExpressionCaptor ExpressionCaptor::operator op(const T &rhs) const  \
    { return append(" " #op " ").appendShow(rhs); }

DEFINE_BINARY_OPERATOR(*)
DEFINE_BINARY_OPERATOR(/)
DEFINE_BINARY_OPERATOR(%)

DEFINE_BINARY_OPERATOR(+)
DEFINE_BINARY_OPERATOR( - )

DEFINE_BINARY_OPERATOR(<<)
DEFINE_BINARY_OPERATOR(>>)

DEFINE_BINARY_OPERATOR(<)
DEFINE_BINARY_OPERATOR(>)
DEFINE_BINARY_OPERATOR(>=)
DEFINE_BINARY_OPERATOR(<=)

DEFINE_BINARY_OPERATOR(==)
DEFINE_BINARY_OPERATOR(!=)

DEFINE_BINARY_OPERATOR(&)

DEFINE_BINARY_OPERATOR(^)

DEFINE_BINARY_OPERATOR(|)

DEFINE_BINARY_OPERATOR(&&)

DEFINE_BINARY_OPERATOR(||)

#undef DEFINE_BINARY_OPERATOR

template<typename T>
ExpressionCaptor ExpressionCaptor::appendShow(const T &value) const
{ return append(toString(value)); }

} // namespace detail
} // namespace rc
