#pragma once

#include <rapidcheck.h>

#include <sstream>

namespace rc {

/// For use with `catch.hpp`. Use this function wherever you would use a
/// `SECTION` for convenient checking of properties.
///
/// @param description  A description of the property.
/// @param testable     The object that implements the property.
template <typename Testable>
void prop(const std::string &description, Testable &&testable) {
  using namespace detail;

  SECTION(description) {
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
      printResultMessage(result, ss);
      INFO(ss.str() << "\n");
      FAIL();
    }
  }
}

} // namespace rc
