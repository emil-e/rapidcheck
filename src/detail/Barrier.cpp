#include "rapidcheck/detail/Barrier.h"

namespace rc {
namespace detail {

Barrier::Barrier(unsigned int count)
    : m_threshold(count)
    , m_count(count)
    , m_generation(0) {
  if (count == 0) {
    throw std::invalid_argument("count cannot be zero.");
  }
}

bool Barrier::wait() {
  std::unique_lock<std::mutex> lock(m_mutex);
  unsigned int gen = m_generation;

  if (--m_count == 0) {
    m_generation++;
    m_count = m_threshold;
    m_cond.notify_all();
    return true;
  }

  while (gen == m_generation)
    m_cond.wait(lock);
  return false;
}

} // namespace detail
} // namespace rc
