#include "rapidcheck/Generator.h"

namespace rc {
namespace gen {

GenerationFailure::GenerationFailure(std::string msg)
    : std::runtime_error(std::move(msg)) {}

int currentSize()
{
    return *detail::ImplicitParam<detail::param::Size>();
}

std::string ValueDescription::typeName() const
{
    if (m_typeInfo == nullptr)
        return std::string();

    return detail::demangle(m_typeInfo->name());
}

std::string ValueDescription::stringValue() const
{ return m_stringValue; }

bool ValueDescription::isNull() const
{
    return m_typeInfo == nullptr;
}

} // namespace gen
} // namespace rc
