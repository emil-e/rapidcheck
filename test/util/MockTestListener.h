#pragma once

#include "rapidcheck/detail/TestListener.h"

namespace rc {
namespace test {

struct MockTestListener : public rc::detail::TestListener {
  void
  onTestCaseFinished(const rc::detail::CaseDescription &description) override {
    if (onTestCaseFinishedCallback) {
      onTestCaseFinishedCallback(description);
    }
  }

  void onShrinkTried(const rc::detail::CaseDescription &shrink,
                     bool accepted) override {
    if (onShrinkTriedCallback) {
      onShrinkTriedCallback(shrink, accepted);
    }
  }

  void onTestFinished(const rc::detail::TestResult &result) override {
    if (onTestFinishedCallback) {
      onTestFinishedCallback(result);
    }
  }

  std::function<void(const rc::detail::CaseDescription &)>
      onTestCaseFinishedCallback;
  std::function<void(const rc::detail::CaseDescription &, bool)>
      onShrinkTriedCallback;
  std::function<void(const rc::detail::TestResult &)> onTestFinishedCallback;
};

} // namespace test
} // namespace rc
