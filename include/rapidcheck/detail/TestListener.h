#pragma once

#include "rapidcheck/detail/Results.h"
#include "rapidcheck/detail/Property.h"

namespace rc {
namespace detail {

/// Interface for classes that listen to events while running properties.
class TestListener {
public:
  /// Called when a test case has finished.
  ///
  /// @param testCase     The test case that was run.
  /// @param description  The case description.
  virtual void onTestCaseFinished(const CaseDescription &description) = 0;

  /// Called when a test case shrink has been tried.
  ///
  /// @param shrink    The shrunk case description.
  /// @param accepted  Whether the shrink was accepted or discarded.
  virtual void onShrinkTried(const CaseDescription &shrink, bool accepted) = 0;

  /// Called when the entire test has finished.
  ///
  /// @param result  The result of the test.
  virtual void onTestFinished(const TestResult &result) = 0;

  virtual ~TestListener() = default;
};

} // namespace detail
} // namespace rc
