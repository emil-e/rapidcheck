#pragma once

#include "rapidcheck/detail/Traits.h"

namespace rc {
namespace predicate {

//! Takes a predicate and creates a predicate which is the complement of that
//! predicate.
template<typename Predicate>
class Not
{
public:
    Not() = default;

    template<typename T>
    Not(T &&predicate);

    template<typename T>
    bool operator()(const T &arg) const;

private:
    Predicate m_predicate;
};

#define DECLARE_BINARY_PREDICATE(Name)                  \
    template<typename T>                                \
    class Name                                          \
    {                                                   \
    public:                                             \
        template<typename U>                            \
        Name(U &&value);                                \
                                                        \
        template<typename U>                            \
        bool operator()(const U &arg) const;            \
                                                        \
    private:                                            \
        T m_value;                                      \
    };

DECLARE_BINARY_PREDICATE(Equals)
DECLARE_BINARY_PREDICATE(GreaterThan)
DECLARE_BINARY_PREDICATE(LessThan)
DECLARE_BINARY_PREDICATE(GreaterEqThan)
DECLARE_BINARY_PREDICATE(LessEqThan)

#undef DECLARE_BINARY_PREDICATE

} // namespace predicate
} // namespace rc

#include "Predicates.hpp"
