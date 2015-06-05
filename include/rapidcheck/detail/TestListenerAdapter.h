#pragma once

#include "rapidcheck/detail/TestListener.h"

namespace rc {
namespace detail {

/// `TestListener` that has no-op default implementations of all methods.
class TestListenerAdapter : public TestListener {
public:
  void onTestCaseFinished(const TestCase &testCase,
                          const CaseDescription &description) {}
  void onShrinkTried(const CaseDescription &shrink, bool accepted) {}
  void onTestFinished(const TestResult &result) {}
};

} // namespace detail
} // namespace rc
