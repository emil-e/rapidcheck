#pragma once

#include "rapidcheck/detail/Results.h"
#include "rapidcheck/detail/Check.h"
#include "rapidcheck/detail/Property.h"

//! TODO move to separate header
#if defined(SECTION) && defined(FAIL)
#  define RC_HAS_CATCH 1
#else
#  define RC_HAS_CATCH 0
#endif

//! Fails the current test case unless the given condition is `true`.
#define RC_ASSERT(condition) RC_CONDITIONAL_RESULT(Failure,             \
                                                   !(condition),        \
                                                   #condition)

//! Unconditionally fails the current test case with the given message.
#define RC_FAIL(msg) RC_CONDITIONAL_RESULT(Failure,                     \
                                           true,                        \
                                           (msg))

//! Succeed if the given condition is true.
#define RC_SUCCEED_IF(condition) RC_CONDITIONAL_RESULT(Success,         \
                                                       (condition),     \
                                                       #condition)

//! Unconditionally succeed with the given message.
#define RC_SUCCEED(msg) RC_CONDITIONAL_RESULT(Success,             \
                                              true,                \
                                              msg)

//! Discards the current test case if the given condition is false.
#define RC_PRE(condition) RC_CONDITIONAL_RESULT(Discard,             \
                                                !(condition),        \
                                                #condition)

//! Discards the current test case with the given description.
#define RC_DISCARD(msg) RC_CONDITIONAL_RESULT(rc::detail::CaseResult::Type::Discard, true, (msg))

namespace rc {

#if RC_HAS_CATCH == 1

//! For use with `catch.hpp`. Use this function wherever you would use a
//! `SECTION` for convenient checking of properties.
//!
//! @param description  A description of the property.
//! @param testable     The object that implements the property.
//!
//! TODO move to separate header
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

#endif

}

#include "detail/Check.hpp"
