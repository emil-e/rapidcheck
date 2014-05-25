#pragma once

#include <cassert>
#include <iostream>
#include <stack>

#include "Utility.hpp"

namespace rc {
namespace detail {

//! Implements something conceptually similar to Lisp dynamic scope or Haskell
//! implicit function parameter. This class lets you bind values to the
//! parameter in a scope and have that value be visible to all readers of the
//! parameter during the execution of that scope. Instantiations of this class
//! are parameterized on tag structs which have a typedef member named
//! \c ValueType which denotes the type of the parameter, for example:
//!
//!     struct MyIntParam { typedef int ValueType; };
//!
template<typename Param>
class ImplicitParam
{
public:
    //! The type of the parameter
    typedef typename Param::ValueType ValueType;

    //! Default constructor
    ImplicitParam<Param>() = default;

    //! Binds a new value to the parameter.
    void let(ValueType value)
    {
        if (m_isBinding)
            values().pop();
        values().push(std::move(value));
        m_isBinding = true;
    }

    //! Checks whether this implicit parameter has a binding.
    bool hasBinding() const
    {
        return !values().empty();
    }

    //! Returns a reference to the bound value. The value can be changed
    //! without introducing a new binding.
    ValueType &operator*()
    {
        assert(hasBinding());
        return values().top();
    }

    //! Returns a const reference to the bound value.
    const ValueType &operator*() const
    {
        assert(hasBinding());
        return values().top();
    }

    //! Member dereference operator for the contained value.
    ValueType *operator->()
    {
        return &(**this);
    }

    //! Removes any introduced binding.
    ~ImplicitParam()
    {
        if (m_isBinding)
            values().pop();
    }

private:
    RC_DISABLE_COPY(ImplicitParam)
    RC_DISABLE_MOVE(ImplicitParam)

    typedef std::stack<ValueType, std::vector<ValueType>> StackT;

    static StackT &values()
    {
        // TODO needs thread local
        static StackT valueStack;
        return valueStack;
    }

    bool m_isBinding = false;
};

//! Outputs the underlying value of the given \c ImplicitParam.
template<typename T>
std::ostream &operator<<(std::ostream& os, const ImplicitParam<T> &p)
{
    os << *p;
    return os;
}

} // namespace detail
} // namespace rc
