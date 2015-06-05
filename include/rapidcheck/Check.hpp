#pragma once

#include <iostream>

#include "rapidcheck/detail/LogTestlistener.h"
#include "rapidcheck/detail/TestParams.h"

namespace rc {
namespace detail {

TestResult checkProperty(const Property &property,
                         const TestParams &params,
                         TestListener &listener);

TestResult checkProperty(const Property &property, const TestParams &params);

template <typename Testable, typename... Args>
TestResult checkTestable(Testable &&testable, Args &&... args) {
  return checkProperty(toProperty(std::forward<Testable>(testable)),
                       std::forward<Args>(args)...);
}

} // namespace detail

template <typename Testable>
bool check(Testable &&testable) {
  return check(std::string(), std::forward<Testable>(testable));
}

template <typename Testable>
bool check(const std::string &description, Testable &&testable) {
  // Force loading of the configuration so that message comes _before_ the
  // description
  detail::defaultTestParams();

  if (!description.empty()) {
    std::cerr << std::endl << "- " << description << std::endl;
  }

  detail::LogTestListener listener(std::cerr);
  const auto result = detail::checkTestable(
      std::forward<Testable>(testable), detail::defaultTestParams(), listener);

  printResultMessage(result, std::cerr);
  std::cerr << std::endl;

  return result.template is<detail::SuccessResult>();
}

} // namespace rc
