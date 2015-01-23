#pragma once

#include <rapidcheck.h>

// TODO remove this and have checkResults in Check.h instead
#include "rapidcheck/detail/Property.h"

namespace rc {

//! For use with `catch.hpp`. Use this function wherever you would use a
//! `SECTION` for convenient checking of properties.
//!
//! @param description  A description of the property.
//! @param testable     The object that implements the property.
template<typename Testable>
void prop(const std::string &description, Testable testable)
{
    using namespace detail;

    SECTION(description) {
        auto result = checkProperty(toProperty(testable));
        INFO(resultMessage(result) << "\n");

        FailureResult failure;
        GaveUpResult gaveUp;
        if (result.match(failure)) {
            std::string counterExample;
            for (const auto &desc : failure.counterExample) {
                counterExample += desc.typeName() + ":\n";
                counterExample += desc.stringValue() + "\n\n";
            }
            INFO(counterExample);
            FAIL("Property failed: " << failure.description);
        } else if (result.match(gaveUp)) {
            INFO(gaveUp.description);
            FAIL("Gave up after " << gaveUp.numTests << " successful tests");
        }
    }
}

} // namespace rc
