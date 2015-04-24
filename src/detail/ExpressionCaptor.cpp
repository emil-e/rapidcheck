#include "rapidcheck/detail/ExpressionCaptor.h"

namespace rc {
namespace detail {

ExpressionCaptor::ExpressionCaptor(std::string stringValue)
    : m_str(stringValue) {}

ExpressionCaptor ExpressionCaptor::append(const std::string &tail) const {
  return ExpressionCaptor(m_str + tail);
}

std::string ExpressionCaptor::str() const { return m_str; }

} // namespace detail
} // namespace rc
