#include "rapidcheck/detail/Any.h"

namespace rc {
namespace detail {

Any::Any() noexcept {}

void Any::reset() { m_impl.reset(); }

std::pair<std::string, std::string> Any::describe() const {
  assert(m_impl);
  return m_impl ? m_impl->describe()
                : std::make_pair(std::string(), std::string());
}

Any::operator bool() const { return static_cast<bool>(m_impl); }

std::ostream &operator<<(std::ostream &os, const Any &value) {
  auto desc = value.describe();
  os << desc.second << " (" << desc.first << ")";
  return os;
}

} // namespace detail
} // namespace rc
