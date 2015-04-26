#pragma once

#include <iostream>

namespace rc {
namespace detail {

template <typename Testable, typename... Args>
TestResult checkTestable(Testable &&testable, const Args &... args) {
  return checkProperty(toProperty(std::forward<Testable>(testable)), args...);
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
  const auto result = detail::checkTestable(std::forward<Testable>(testable));
  printResultMessage(result, std::cerr);
  std::cerr << std::endl;
  return result.template is<detail::SuccessResult>();
}

} // namespace rc
