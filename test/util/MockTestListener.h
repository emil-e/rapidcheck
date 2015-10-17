#pragma once

#include "rapidcheck/detail/TestListener.h"

namespace rc {
namespace test {

struct MockTestListener : public rc::detail::TestListener {
  void
  onTestCaseFinished(const rc::detail::CaseDescription &description) override {
    onTestCaseFinishedCount++;
    if (onTestCaseFinishedCallback) {
      onTestCaseFinishedCallback(description);
    }
  }

  void onShrinkTried(const rc::detail::CaseDescription &shrink,
                     bool accepted) override {
    onShrinkTriedCount++;
    if (onShrinkTriedCallback) {
      onShrinkTriedCallback(shrink, accepted);
    }
  }

  void onTestFinished(const rc::detail::TestMetadata &metadata,
                      const rc::detail::TestResult &result) override {
    onTestFinishedCount++;
    if (onTestFinishedCallback) {
      onTestFinishedCallback(metadata, result);
    }
  }

  std::function<void(const rc::detail::CaseDescription &)>
      onTestCaseFinishedCallback;
  int onTestCaseFinishedCount = 0;

  std::function<void(const rc::detail::CaseDescription &, bool)>
      onShrinkTriedCallback;
  int onShrinkTriedCount = 0;

  std::function<void(const rc::detail::TestMetadata &,
                     const rc::detail::TestResult &)> onTestFinishedCallback;
  int onTestFinishedCount = 0;
};

} // namespace test
} // namespace rc
