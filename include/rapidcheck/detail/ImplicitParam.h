#pragma once

#include <stack>
#include <vector>

#include "Utility.h"

namespace rc {
namespace detail {

//! Implements something conceptually similar to Lisp dynamic scope or Haskell
//! implicit function parameters. This class lets you bind values to the
//! parameter in a scope and have that value be visible to all readers of the
//! parameter during the execution of that scope. Instantiations of this class
//! are parameterized on tag structs which have a typedef member named
//! \c ValueType which denotes the type of the parameter and a static method
//! `defaultValue` which returns the default value. For example:
//!
//!     struct MyIntParam
//!     {
//!         typedef int ValueType;
//!         static int defaultValue() { return 1337; }
//!     };
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
    void let(ValueType value);

    //! Returns a reference to the bound value. The value can be changed
    //! without introducing a new binding.
    ValueType &operator*();

    //! Returns a const reference to the bound value.
    const ValueType &operator*() const;

    //! Member dereference operator. Only useful if `ValueType` is pointer-like.
    ValueType &operator->();

    //! Const version of member dereference.
    const ValueType &operator->() const;

    //! Removes any introduced binding.
    ~ImplicitParam();

private:
    RC_DISABLE_COPY(ImplicitParam)

    typedef std::stack<ValueType, std::vector<ValueType>> StackT;

    static StackT &values();

    bool m_isBinding = false;
};

//! Outputs the underlying value of the given \c ImplicitParam.
template<typename T>
std::ostream &operator<<(std::ostream& os, const ImplicitParam<T> &p);

} // namespace detail
} // namespace rc

#include "ImplicitParam.hpp"
