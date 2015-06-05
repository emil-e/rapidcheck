#pragma once

#include <iostream>

#include "rapidcheck/detail/TestListener.h"

namespace rc {
namespace detail {

/// Listener which prints info to an `std::ostream`.
class LogTestListener : public TestListener {
public:
  explicit LogTestListener(std::ostream &os);
  void onTestCaseFinished(const CaseDescription &description) override;
  void onShrinkTried(const CaseDescription &shrink, bool accepted) override;
  void onTestFinished(const TestResult &result) override;

private:
  std::ostream &m_out;
};

} // namespace detail
} // namespace rc
