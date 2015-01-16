#pragma once

#include "Traits.h"

namespace rc {
namespace detail {

namespace {

template<typename T>
void deleteValue(void *p) { delete static_cast<T *>(p); }

template<typename T>
ValueDescription describeValue(void *p)
{ return ValueDescription(*static_cast<T *>(p)); }

}

template<typename T>
Any::Any(T &&value)
    : m_value(new T(std::forward<T>(value)))
    , m_delete(deleteValue<DecayT<T>>)
    , m_describe(describeValue<DecayT<T>>) {}

template<typename T>
Any &Any::operator=(T &&rhs)
{
    m_delete(m_value);
    m_value = new DecayT<T>(std::forward<T>(rhs));
    m_delete = deleteValue<DecayT<T>>;
    m_describe = describeValue<DecayT<T>>;
    return *this;
}

template<typename T>
const T &Any::get() const { return *static_cast<T *>(m_value); }

template<typename T>
T &Any::get() { return *static_cast<T *>(m_value); }

} // namespace detail
} // namespace rc
