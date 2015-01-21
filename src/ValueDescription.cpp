#include "rapidcheck/detail/ValueDescription.h"

#include <string>

namespace rc {
namespace detail {

std::string ValueDescription::typeName() const
{ return m_typeName; }

std::string ValueDescription::stringValue() const
{ return m_stringValue; }

bool ValueDescription::isNull() const
{ return m_typeName.empty(); }

bool ValueDescription::operator==(const ValueDescription &rhs) const
{
    return
        (m_typeName == rhs.m_typeName) &&
        (m_stringValue == rhs.m_stringValue);
}

bool ValueDescription::operator!=(const ValueDescription &rhs) const
{
    return !(*this == rhs);
}

std::ostream &operator<<(std::ostream &os, const ValueDescription &value)
{
    if (value.isNull())
        os << "<null>";
    else
        os << value.stringValue() << " (" << value.typeName() << ")";
    return os;
}

} // namespace detail
} // namespace rc
