#pragma once

#include <vector>
#include <map>

#include "Variant.h"

namespace rc {
namespace detail {

using Example = std::vector<std::pair<std::string, std::string>>;
using Tags = std::vector<std::string>;
using Distribution = std::map<Tags, int>;

/// Describes the result of a test case.
struct CaseResult {
  /// Enum for the type of the result.
  enum class Type {
    /// The test case succeeded.
    Success,
    /// The test case failed.
    Failure,
    /// The preconditions for the test case were not met.
    Discard
  };

  CaseResult();
  CaseResult(Type t, std::string desc);

  /// The type of the result.
  Type type;

  /// A description of the result.
  std::string description;
};

std::ostream &operator<<(std::ostream &os, CaseResult::Type type);
std::ostream &operator<<(std::ostream &os, const CaseResult &result);
bool operator==(const CaseResult &r1, const CaseResult &r2);
bool operator!=(const CaseResult &r1, const CaseResult &r2);
bool operator<(const CaseResult &r1, const CaseResult &r2);

/// Indicates a successful property.
struct SuccessResult {
  /// The number of successful tests run.
  int numSuccess;
  /// The test case distribution. This is a map from tags to count.
  Distribution distribution;
};

std::ostream &operator<<(std::ostream &os, const detail::SuccessResult &result);
bool operator==(const SuccessResult &r1, const SuccessResult &r2);
bool operator!=(const SuccessResult &r1, const SuccessResult &r2);

/// Indicates that a property failed.
struct FailureResult {
  /// The number of successful tests run.
  int numSuccess;
  /// A description of the failure.
  std::string description;
  /// The number of shrinks performed.
  int numShrinks;
  /// The counterexample.
  Example counterExample;
};

std::ostream &operator<<(std::ostream &os, const detail::FailureResult &result);
bool operator==(const FailureResult &r1, const FailureResult &r2);
bool operator!=(const FailureResult &r1, const FailureResult &r2);

/// Indicates that more test cases than allowed were discarded.
struct GaveUpResult {
  /// The number of successful tests run.
  int numSuccess;
  /// A description of the reason for giving up.
  std::string description;
};

std::ostream &operator<<(std::ostream &os, const detail::GaveUpResult &result);
bool operator==(const GaveUpResult &r1, const GaveUpResult &r2);
bool operator!=(const GaveUpResult &r1, const GaveUpResult &r2);

/// Describes the circumstances around the result of a test.
using TestResult = Variant<SuccessResult, FailureResult, GaveUpResult>;

/// Prints a human readable error message describing the given success result to
/// the specified stream.
void printResultMessage(const SuccessResult &result, std::ostream &os);

/// Prints a human readable error message describing the given failure result to
/// the specified stream.
void printResultMessage(const FailureResult &result, std::ostream &os);

/// Prints a human readable error message describing the given gave-up result to
/// the specified stream.
void printResultMessage(const GaveUpResult &result, std::ostream &os);

/// Prints a short message describing the given test results to the specified
/// stream.
void printResultMessage(const TestResult &result, std::ostream &os);

} // namespace detail
} // namespace rc
