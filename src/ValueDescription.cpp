#include "rapidcheck/detail/ValueDescription.h"

namespace rc {
namespace detail {

std::string ValueDescription::typeName() const
{ return m_typeName; }

std::string ValueDescription::stringValue() const
{ return m_stringValue; }

bool ValueDescription::isNull() const
{ return m_typeName.empty(); }

} // namespace detail
} // namespace rc
