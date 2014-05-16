#pragma once

#include "Show.hpp"
#include "Shrink.hpp"

namespace rc {
namespace detail {

//! Describes a value and its type
class ValueDescription
{
public:
    template<typename T>
    ValueDescription(const T &value)
        : m_typeInfo(&typeid(T))
    {
        std::ostringstream ss;
        show(value, ss);
        m_stringValue = ss.str();
    }

    //! Returns a string describing the type of the value.
    std::string typeName() const
    { return demangle(m_typeInfo->name()); }

    //! Returns a string representation of the value.
    std::string value() const
    { return m_stringValue; }

private:
    const std::type_info *m_typeInfo;
    std::string m_stringValue;
};

//! Since a lot of code is templated on the specific generator or the generated
//! type, we need a way to work with generators after we have returned from the
//! templated context where the type information is lost. This class in
//! combination with the \c TypedGeneratorProxy subclass lets us do that.
class GeneratorProxy
{
public:
    //! Generates the value and returns a \c ValueDescription.
    virtual ValueDescription describe() const = 0;
};

//! Concrete implementation of \c GeneratorProxy and also the class which contain
//! of the type specific operations.
template<typename T>
class TypedGeneratorProxy : public GeneratorProxy, public Generator<T>
{
public:
    //! Constructs a new proxy with the given underlying generator.
    template<typename Gen>
    TypedGeneratorProxy(Gen generator)
        : m_generator(new Gen(std::move(generator))) {}

    T operator()() const override { return (*m_generator)(); }

    ValueDescription describe() const override
    {
        return ValueDescription((*this)());
    }

private:
    RC_DISABLE_COPY(TypedGeneratorProxy)

    GeneratorUP<T> m_generator;
};

typedef std::unique_ptr<GeneratorProxy> GeneratorProxyUP;

} // namespace detail
} // namespace rc
