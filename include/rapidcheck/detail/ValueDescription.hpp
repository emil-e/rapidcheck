#pragma once

#include <sstream>

#include "rapidcheck/Show.h"

namespace rc {
namespace detail {

template<typename T>
ValueDescription::ValueDescription(const T &value)
    : m_typeInfo(&typeid(T))
{
    std::ostringstream ss;
    show(value, ss);
    m_stringValue = ss.str();
}

} // namespace detail
} // namespace rc
