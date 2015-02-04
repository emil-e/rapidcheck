#include "rapidcheck/detail/Any.h"

namespace rc {
namespace detail {

Any::Any() {}

Any::Any(const Any &other)
    : m_impl(other.m_impl
             ? other.m_impl->copy()
             : nullptr) {}

Any::Any(Any &&other)
    : m_impl(other.m_impl.release()) {}

Any &Any::operator=(const Any &rhs)
{
    if (rhs.m_impl)
        m_impl = rhs.m_impl->copy();
    else
        m_impl.reset();
    return *this;
}

Any &Any::operator=(Any &&rhs)
{
    m_impl.reset(rhs.m_impl.release());
    return *this;
}

void Any::reset() { m_impl.reset(); }

std::pair<std::string, std::string> Any::describe() const
{
    assert(m_impl);
    return m_impl
        ? m_impl->describe()
        : std::make_pair(std::string(), std::string());
}

Any::operator bool() const { return static_cast<bool>(m_impl); }

bool Any::isCopyable() const { return !m_impl || m_impl->isCopyable(); }

} // namespace detail
} // namespace rc
