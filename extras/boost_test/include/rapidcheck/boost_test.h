#pragma once

#include <rapidcheck.h>

namespace rc {
namespace detail {

template <typename Testable>
void checkBoostTest(const std::string &description, Testable &&testable) {
  const auto result = checkTestable(std::forward<Testable>(testable));

  if (result.template is<SuccessResult>()) {
    const auto success = result.template get<SuccessResult>();
    if (!success.distribution.empty()) {
      std::cout << "- " << description << std::endl;
      printResultMessage(result, std::cout);
      std::cout << std::endl;
    }
  } else {
    std::ostringstream ss;
    ss << std::endl;
    printResultMessage(result, ss);
    BOOST_FAIL(ss.str());
  }
}

} // namespace detail
} // namespace rc

#define RC_BOOST_PROP(Name, ArgList)                                           \
  void rapidCheckPropImpl_##Name ArgList;                                      \
                                                                               \
  BOOST_AUTO_TEST_CASE(Name) {                                                 \
    ::rc::detail::checkBoostTest(#Name, &rapidCheckPropImpl_##Name);           \
  }                                                                            \
                                                                               \
  void rapidCheckPropImpl_##Name ArgList
