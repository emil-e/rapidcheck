#include "rapidcheck/detail/ValueDescription.h"

#include <string>

namespace rc {
namespace detail {

ValueDescription::ValueDescription(std::string typeName,
                                   std::string stringValue)
    : m_typeName(typeName)
    , m_stringValue(stringValue) {}

std::string ValueDescription::typeName() const
{ return m_typeName; }

std::string ValueDescription::stringValue() const
{ return m_stringValue; }

bool ValueDescription::isNull() const
{ return m_typeName.empty(); }

bool operator==(const ValueDescription &lhs, const ValueDescription &rhs)
{
    return
        (lhs.typeName() == rhs.typeName()) &&
        (lhs.stringValue() == rhs.stringValue());
}

bool operator!=(const ValueDescription &lhs, const ValueDescription &rhs)
{ return !(lhs == rhs); }

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
