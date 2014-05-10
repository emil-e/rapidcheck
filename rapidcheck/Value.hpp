#pragma once

#include "Show.hpp"

namespace rc {
namespace detail {

//! Classes implementing this interface are used to store generated values
//! so that they can be shrunk and displayed to the user on a failing property.
class Value
{
public:
    //! Outputs a representation of the represented value to the given output
    //! stream.
    //!
    //! @param os  The output stream.
    virtual void show(std::ostream &os) const = 0;

    //! Returns the type_info for the represented value.
    virtual const std::type_info &typeInfo() const = 0;

    virtual ~Value() = default;
};

//! Subclass template of \c Value which also provides means to retrieve a copy
//! of the represented value. Parameterized on the type of the represented value.
template<typename T>
class TypedValue : public Value
{
public:
    //! Returns a copy of the represented value.
    virtual T get() const = 0;
};

//! Implementation of \c TypedValue which simply stores a copy of the
//! represented value.
template<typename T>
class StoredValue : public TypedValue<T>
{
public:
    StoredValue(T value) : m_value(std::move(value)) {}

    void show(std::ostream &os) const override
    { rc::show(m_value, os); }

    const std::type_info &typeInfo() const override
    { return typeid(m_value); }

    T get() const override { return m_value; }

private:
    T m_value;
};

typedef std::unique_ptr<Value> ValueUP;

template<typename T>
using TypedValueUP = std::unique_ptr<TypedValue<T>>;

} // namespace detail
} // namespace rc
