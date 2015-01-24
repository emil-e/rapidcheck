#pragma once

#include <stack>
#include <vector>

#include "Utility.h"

namespace rc {
namespace detail {

// TODO separate into h/hpp files

//! Implements something conceptually similar to Lisp dynamic scope or Haskell
//! implicit function parameters. This class lets you bind values to the
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

    //! Returns a reference to the bound value. The value can be changed
    //! without introducing a new binding.
    ValueType &operator*()
    {
        return values().top();
    }

    //! Returns a const reference to the bound value.
    const ValueType &operator*() const
    {
        return values().top();
    }

    //! Member dereference operator. Only useful if `ValueType` is pointer-like.
    ValueType &operator->()
    {
        return **this;
    }

    //! Const version of member dereference.
    const ValueType &operator->() const
    {
        return **this;
    }

    //! Removes any introduced binding.
    ~ImplicitParam()
    {
        if (m_isBinding)
            values().pop();
    }

private:
    RC_DISABLE_COPY(ImplicitParam)

    typedef std::stack<ValueType, std::vector<ValueType>> StackT;

    static StackT &values()
    {
        // TODO needs thread local
        static StackT valueStack;
        if (valueStack.empty())
            valueStack.push(Param::defaultValue());
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
