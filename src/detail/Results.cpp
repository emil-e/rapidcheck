#include "rapidcheck/detail/Results.h"

#include <iomanip>
#include <algorithm>

#include "rapidcheck/Show.h"

namespace rc {
namespace detail {

CaseResult::CaseResult()
    : type(CaseResult::Type::Failure) {}

CaseResult::CaseResult(Type t, std::string desc)
    : type(t)
    , description(desc) {}

bool operator==(const CaseResult &r1, const CaseResult &r2) {
  return (r1.type == r2.type) && (r1.description == r2.description);
}

bool operator!=(const CaseResult &r1, const CaseResult &r2) {
  return !(r1 == r2);
}

bool operator==(const SuccessResult &r1, const SuccessResult &r2) {
  return (r1.numSuccess == r2.numSuccess) &&
      (r1.distribution == r2.distribution);
}

bool operator!=(const SuccessResult &r1, const SuccessResult &r2) {
  return !(r1 == r2);
}

std::ostream &operator<<(std::ostream &os,
                         const detail::SuccessResult &result) {
  os << "numSuccess=" << result.numSuccess << ", distribution=";
  show(result.distribution, os);
  return os;
}

bool operator==(const FailureResult &r1, const FailureResult &r2) {
  return (r1.numSuccess == r2.numSuccess) &&
      (r1.description == r2.description) && (r1.numShrinks == r2.numShrinks) &&
      (r1.counterExample == r2.counterExample);
}

bool operator!=(const FailureResult &r1, const FailureResult &r2) {
  return !(r1 == r2);
}

std::ostream &operator<<(std::ostream &os,
                         const detail::FailureResult &result) {
  os << "numSuccess=" << result.numSuccess << ", description='"
     << result.description << "'"
     << ", numShrinks=" << result.numShrinks << ", counterExample=";
  show(result.counterExample, os);
  return os;
}

bool operator==(const GaveUpResult &r1, const GaveUpResult &r2) {
  return (r1.numSuccess == r2.numSuccess) && (r1.description == r2.description);
}

bool operator!=(const GaveUpResult &r1, const GaveUpResult &r2) {
  return !(r1 == r2);
}

std::ostream &operator<<(std::ostream &os, const detail::GaveUpResult &result) {
  os << "numSuccess=" << result.numSuccess << ", description='"
     << result.description << "'";
  return os;
}

void printDistribution(const SuccessResult &result, std::ostream &os) {
  using Entry = std::pair<Tags, int>;
  std::vector<Entry> entries(begin(result.distribution),
                             end(result.distribution));

  std::sort(
      begin(entries),
      end(entries),
      [](const Entry &p1, const Entry &p2) { return p2.second < p1.second; });

  for (const auto &entry : entries) {
    const auto percent =
        (static_cast<double>(entry.second) / result.numSuccess) * 100.0;
    os << std::setw(5) << std::setprecision(2) << std::fixed << percent
       << "% - ";
    for (auto it = begin(entry.first); it != end(entry.first); it++) {
      if (it != begin(entry.first)) {
        os << ", ";
      }
      os << *it;
    }
    os << std::endl;
  }
}

void printResultMessage(const SuccessResult &result, std::ostream &os) {
  os << "OK, passed " + std::to_string(result.numSuccess) + " tests";
  if (!result.distribution.empty()) {
    os << std::endl;
    printDistribution(result, os);
  }
}

void printResultMessage(const FailureResult &result, std::ostream &os) {
  os << "Falsifiable after " << (result.numSuccess + 1);
  os << " tests";
  if (result.numShrinks > 0) {
    os << " and " << result.numShrinks << " shrink";
    if (result.numShrinks > 1) {
      os << 's';
    }
  }

  os << std::endl << std::endl;

  for (const auto &item : result.counterExample) {
    os << item.first << ":" << std::endl;
    os << item.second << std::endl;
    os << std::endl;
  }

  os << result.description;
}

void printResultMessage(const GaveUpResult &result, std::ostream &os) {
  os << "Gave up after " << result.numSuccess << " tests" << std::endl;
  os << std::endl;
  os << result.description;
}

void printResultMessage(const TestResult &result, std::ostream &os) {
  SuccessResult success;
  FailureResult failure;
  GaveUpResult gaveUp;

  // TODO Make Variant cooler!
  if (result.match(success)) {
    printResultMessage(success, os);
  } else if (result.match(failure)) {
    printResultMessage(failure, os);
  } else if (result.match(gaveUp)) {
    printResultMessage(gaveUp, os);
  }
}

std::ostream &operator<<(std::ostream &os, CaseResult::Type type) {
  switch (type) {
  case CaseResult::Type::Success:
    os << "Success";
    break;

  case CaseResult::Type::Failure:
    os << "Failure";
    break;

  case CaseResult::Type::Discard:
    os << "Discard";
    break;
  }

  return os;
}

std::ostream &operator<<(std::ostream &os, const CaseResult &result) {
  os << result.type << ": " << result.description;
  return os;
}

} // namespace detail
} // namespace rc
