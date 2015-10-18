#include "DefaultTestListener.h"

#include "LogTestListener.h"

namespace rc {
namespace detail {

std::unique_ptr<TestListener>
makeDefaultTestListener(const Configuration &config, std::ostream &os) {
  return std::unique_ptr<TestListener>(
      new LogTestListener(os, config.verboseProgress, config.verboseShrinking));
}

TestListener &globalTestListener() {
  static const auto listener =
      makeDefaultTestListener(configuration(), std::cerr);
  return *listener;
}

} // namespace detail
} // namespace rc
