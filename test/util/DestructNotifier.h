#pragma once

#include <string>
#include <vector>

namespace rc {
namespace test {

/// Test utility for detecting that a destructor is called.
class DestructNotifier {
public:
  DestructNotifier(std::string id, std::vector<std::string> *logp)
      : m_id(std::move(id))
      , m_logp(logp) {}

  DestructNotifier(const DestructNotifier &) = default;

  /// Move steals the pointer.
  DestructNotifier(DestructNotifier &&other)
      : m_id(std::move(other.m_id))
      , m_logp(other.m_logp) {
    other.m_logp = nullptr;
  }

  DestructNotifier &operator=(const DestructNotifier &) = delete;
  DestructNotifier &operator=(DestructNotifier &&) = delete;

  std::string id() const { return m_id; }

  ~DestructNotifier() {
    if (m_logp != nullptr)
      m_logp->push_back(m_id);
  }

private:
  std::string m_id;
  std::vector<std::string> *m_logp;
};

} // namespace test
} // namespace rc
