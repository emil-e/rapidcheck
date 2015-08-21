#pragma once

#include <stdexcept>
#include <mutex>
#include <condition_variable>

namespace rc {
namespace detail {

/// Syncronizes multiple threads
class Barrier {
public:
  Barrier(unsigned int count);

  bool wait();

private:
  std::mutex m_mutex;
  std::condition_variable m_cond;
  unsigned int m_threshold;
  unsigned int m_count;
  unsigned int m_generation;
};

} // namespace detail
} // namespace rc
