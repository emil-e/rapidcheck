#pragma once

#include <sstream>

#include "rapidcheck/Show.h"

#include "ShowType.h"

namespace rc {
namespace detail {

template<typename T>
ValueDescription::ValueDescription(const T &value)
{
    std::ostringstream stringValue;
    show(value, stringValue);
    m_stringValue = stringValue.str();

    std::ostringstream typeName;
    showType<T>(typeName);
    m_typeName = typeName.str();
}

} // namespace detail
} // namespace rc
