#pragma once

#include "detail/GeneratorFwd.h"
#include "detail/Results.h"
#include "detail/Property.h"

#if defined(SECTION) && defined(FAIL)
#  define RC_HAS_CATCH 1
#else
#  define RC_HAS_CATCH 0
#endif

//! Falsifies the current property unless `condition` is `true`.
#define RC_ASSERT(condition)                                    \
    ::rc::detail::assertTrue((condition), #condition, __FILE__, __LINE__)

namespace rc {
namespace detail {

//! Checks the given property and returns the results.
TestResult checkProperty(const gen::GeneratorUP<CaseResult> &property);

//! Throws failure if `condition` is false.
//!
//! @param condition    The condition to check.
//! @param description  A description of the potential failure.
//! @param file         The file in which the failure occurs.
//! @param line         The line at which the failure occurs.
void assertTrue(bool condition,
                std::string description,
                std::string file,
                int line);

} // namespace detail

#if RC_HAS_CATCH == 1

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
        INFO(resultMessage(result));

        FailureResult failure;
        if (result.match(failure))
            FAIL("Property failed: " << failure.description);
    }
}

#endif

}

#include "detail/Check.hpp"
