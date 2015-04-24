#pragma once

#include <string>

namespace rc {
namespace detail {

/// Utility class capturing an expression into a string. Mainly for use in
/// assertion macros and the likes.
class ExpressionCaptor
{
public:
    explicit ExpressionCaptor(std::string stringValue = std::string());

    template<typename T>
    ExpressionCaptor operator->*(const T &rhs) const;

#define DECLARE_BINARY_OPERATOR(op)                     \
    template<typename T>                                \
    ExpressionCaptor operator op(const T &rhs) const;

    DECLARE_BINARY_OPERATOR(*)
    DECLARE_BINARY_OPERATOR(/)
    DECLARE_BINARY_OPERATOR(%)

    DECLARE_BINARY_OPERATOR(+)
    DECLARE_BINARY_OPERATOR( - )

    DECLARE_BINARY_OPERATOR(<<)
    DECLARE_BINARY_OPERATOR(>>)

    DECLARE_BINARY_OPERATOR(<)
    DECLARE_BINARY_OPERATOR(>)
    DECLARE_BINARY_OPERATOR(>=)
    DECLARE_BINARY_OPERATOR(<=)

    DECLARE_BINARY_OPERATOR(==)
    DECLARE_BINARY_OPERATOR(!=)

    DECLARE_BINARY_OPERATOR(&)

    DECLARE_BINARY_OPERATOR(^)

    DECLARE_BINARY_OPERATOR(|)

    DECLARE_BINARY_OPERATOR(&&)

    DECLARE_BINARY_OPERATOR(||)

#undef DECLARE_BINARY_OPERATOR

    std::string str() const;

private:
    ExpressionCaptor append(const std::string &tail) const;

    template<typename T>
    ExpressionCaptor appendShow(const T &value) const;

    std::string m_str;
};

} // namespace detail
} // namespace rc

#include "ExpressionCaptor.hpp"
