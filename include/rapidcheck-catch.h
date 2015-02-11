#pragma once

#include <rapidcheck.h>

namespace rc {

//! For use with `catch.hpp`. Use this function wherever you would use a
//! `SECTION` for convenient checking of properties.
//!
//! @param description  A description of the property.
//! @param testable     The object that implements the property.
template<typename Testable>
void prop(const std::string &description, const Testable &testable)
{
    using namespace detail;

    SECTION(description) {
        auto result = checkTestable(testable);
        INFO(resultMessage(result) << "\n");

        FailureResult failure;
        GaveUpResult gaveUp;
        if (result.match(failure)) {
            std::string counterExample;
            for (const auto &desc : failure.counterExample) {
                counterExample += desc.first + ":\n";
                counterExample += desc.second + "\n\n";
            }
            INFO(counterExample);
            FAIL(failure.description);
        } else if (result.match(gaveUp)) {
            INFO(gaveUp.description);
            FAIL("Gave up after " << gaveUp.numSuccess << " successful tests");
        }
    }
}

} // namespace rc
