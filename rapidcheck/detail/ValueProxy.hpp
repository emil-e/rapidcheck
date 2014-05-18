#pragma once

#include "rapidcheck/Generator.h"
#include "rapidcheck/Show.hpp"
#include "rapidcheck/Shrink.h"

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
//! type, we need a way to work with values and generators after we have
//! returned from the templated context where the type information is lost. This
//! class in combination with the \c TypedValueProxy subclass lets us do
//! that.
//!
//! Please note that this class must always be used in the context of a
//! \c RoseNode since values may actually be regenerated.
class ValueProxy
{
public:
    //! Returns a \c ValueDescription for the value that is generated.
    virtual ValueDescription describe() const = 0;

    //! Overrides the generated value with the next possible shrink if there is
    //! one. If all alternatives have been exhausted, the original value will be
    //! used again.
    //!
    //! @return \c true if successfully shrunk, \c false if there are no more
    //!         shrinks
    virtual bool nextShrink() = 0;

    //! Accepts the current shrink and thus ends the shrinking process.
    //! The accepted value will override the generated value until further
    //! shrinking is performed.
    virtual void acceptShrink() = 0;

    //! Returns the type name of the underlying generator.
    virtual std::string typeName() const = 0;

    virtual ~ValueProxy() = default;
};

//! Concrete implementation of \c ValueProxy and also the class which contain
//! of the type specific operations.
template<typename T>
class TypedValueProxy : public ValueProxy, public Generator<T>
{
public:
    //! Constructs a new proxy with the given underlying generator.
    template<typename Gen>
    TypedValueProxy(Gen generator)
        : m_generator(new Gen(std::move(generator))) {}

    T operator()() const override
    {
        if (m_currentShrink)
            return *m_currentShrink;
        else if (m_acceptedShrink)
            return *m_acceptedShrink;
        else
            return (*m_generator)();
    }

    ValueDescription describe() const override
    {
        return ValueDescription((*this)());
    }

    bool nextShrink() override
    {
        // If all shrinks are already exhausted
        if (m_shrinkExhausted)
            return false;

        // Shrink iterator is created lazily
        if (!m_shrinkIterator)
            m_shrinkIterator = m_generator->shrink((*this)());

        if (m_shrinkIterator->hasNext()) {
            // Don't reallocate unnecessarily
            if (!m_currentShrink) {
                m_currentShrink = std::unique_ptr<T>(
                    new T(m_shrinkIterator->next()));
            } else {
                *m_currentShrink = m_shrinkIterator->next();
            }
            return true;
        } else {
            // These won't be needed anymore
            m_currentShrink = nullptr;
            m_shrinkIterator = nullptr;
            m_shrinkExhausted = true;
            return false;
        }
    }

    void acceptShrink() override
    {
        m_shrinkIterator = nullptr;
        m_acceptedShrink = std::move(m_currentShrink);
    }

    std::string typeName() const
    { return demangle(typeid(*m_generator).name()); }

private:
    RC_DISABLE_COPY(TypedValueProxy)

    GeneratorUP<T> m_generator;
    ShrinkIteratorUP<T> m_shrinkIterator;
    std::unique_ptr<T> m_currentShrink;
    std::unique_ptr<T> m_acceptedShrink;
    bool m_shrinkExhausted = false;
};

typedef std::unique_ptr<ValueProxy> ValueProxyUP;

} // namespace detail
} // namespace rc
