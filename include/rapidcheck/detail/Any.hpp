#pragma once

#include <cassert>

#include "rapidcheck/Show.h"
#include "Utility.h"
#include "Traits.h"
#include "ShowType.h"

namespace rc {
namespace detail {

class AbstractAnyImpl
{
public:
    virtual void *get() = 0;
    virtual bool isCopyable() const = 0;
    virtual std::unique_ptr<AbstractAnyImpl> copy() const = 0;
    virtual std::pair<std::string, std::string> describe() const = 0;
    virtual const std::type_info &typeInfo() const = 0;
    virtual ~AbstractAnyImpl() = default;
};

template<typename T>
class AnyImpl : public AbstractAnyImpl
{
public:
    template<typename ValueT>
    AnyImpl(ValueT &&value)
        : m_value(std::forward<ValueT>(value)) {}

    void *get() override { return &m_value; }

    bool isCopyable() const override { return IsCopyConstructible<T>::value; }

    std::unique_ptr<AbstractAnyImpl> copy() const override
    { return copy(IsCopyConstructible<T>()); }

    std::pair<std::string, std::string> describe() const override
    {
        return { typeToString<T>(), toString(m_value) };
    }

    const std::type_info &typeInfo() const override
    { return typeid(T); }

private:
    RC_DISABLE_COPY(AnyImpl)

    std::unique_ptr<AbstractAnyImpl> copy(std::true_type) const
    { return std::unique_ptr<AbstractAnyImpl>(new AnyImpl<T>(m_value)); }

    // TODO better error message
    std::unique_ptr<AbstractAnyImpl> copy(std::false_type) const
    { throw std::runtime_error("Not copyable"); }

    T m_value;
};

//! Constructs a new `Any` with the given value.
template<typename T>
Any Any::of(T &&value)
{
    Any any;
    any.m_impl.reset(new AnyImpl<DecayT<T>>(std::forward<T>(value)));
    return any;
}

template<typename T>
const T &Any::get() const
{
    assert(m_impl);
    assert(m_impl->typeInfo() == typeid(T));
    return *static_cast<T *>(m_impl->get());
}

template<typename T>
T &Any::get()
{
    assert(m_impl);
    assert(m_impl->typeInfo() == typeid(T));
    return *static_cast<T *>(m_impl->get());
}

} // namespace detail
} // namespace rc
