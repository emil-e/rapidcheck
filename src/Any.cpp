#include "rapidcheck/detail/Any.h"

namespace rc {
namespace detail {

Any::Any() { reset(); }

Any::Any(Any &&other)
    : m_value(other.m_value)
    , m_delete(other.m_delete)
    , m_describe(other.m_describe)
{ other.m_value = nullptr; }

void Any::reset()
{
    m_value = nullptr;
    m_delete = [](void *){};
    m_describe = [](void *){ return ValueDescription(); };
}

Any &Any::operator=(Any &&rhs)
{
    m_delete(m_value);
    m_value = rhs.m_value;
    m_delete = rhs.m_delete;
    m_describe = rhs.m_describe;
    rhs.m_value = nullptr;
    return *this;
}

ValueDescription Any::describe() const { return m_describe(m_value); }

Any::operator bool() const { return m_value != nullptr; }

Any::~Any() { m_delete(m_value); }

} // namespace detail
} // namespace rc
