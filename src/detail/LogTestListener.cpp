#include "rapidcheck/detail/LogTestListener.h"

#include "rapidcheck/detail/Property.h"

namespace rc {
namespace detail {

LogTestListener::LogTestListener(std::ostream &os)
    : m_out(os) {}

void LogTestListener::onTestCaseFinished(const TestCase &testCase,
                                         const CaseDescription &description) {
  switch (description.result.type) {
  case CaseResult::Type::Success:
    m_out << ".";
    break;
  case CaseResult::Type::Discard:
    m_out << "x";
    break;
  case CaseResult::Type::Failure:
    m_out << std::endl << "Found failure, shrinking:" << std::endl;
    break;
  }
}

void LogTestListener::onShrinkTried(const CaseDescription &shrink,
                                    bool accepted) {
  if (accepted) {
    m_out << "!";
  } else {
    m_out << ".";
  }
}

void LogTestListener::onTestFinished(const TestResult &result) {
  m_out << std::endl;
}

} // namespace detail
} // namespace rc
