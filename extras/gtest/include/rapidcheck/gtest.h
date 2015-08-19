#pragma once

#include <rapidcheck.h>

namespace rc {
namespace detail {

template <typename Testable>
void checkGtest(Testable &&testable) {
  const auto result = checkTestable(std::forward<Testable>(testable));

  if (result.template is<SuccessResult>()) {
    const auto success = result.template get<SuccessResult>();
    if (!success.distribution.empty()) {
      printResultMessage(result, std::cout);
      std::cout << std::endl;
    }
  } else {
    std::ostringstream ss;
    printResultMessage(result, ss);
    FAIL() << ss.str() << std::endl;
  }
}

} // namespace detail
} // namespace rc

#define RC_GTEST_PROP(TestCase, Name, ArgList)                                 \
  void rapidCheckPropImpl_##TestCase##_##Name ArgList;                         \
                                                                               \
  TEST(TestCase, Name) {                                                       \
    ::rc::detail::checkGtest(&rapidCheckPropImpl_##TestCase##_##Name);         \
  }                                                                            \
                                                                               \
  void rapidCheckPropImpl_##TestCase##_##Name ArgList
