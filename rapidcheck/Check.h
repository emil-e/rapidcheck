#pragma once

namespace rc {

//! Describes the parameters for a test.
struct TestParameters
{
    //! The maximum number of successes before deciding a property passes.
    int maxSuccess = 100;
    //! The maximum size to generate.
    size_t maxSize = 100;
};

//TODO document when done
template<typename Testable>
bool check(Testable testable);

}

#include "detail/Check.hpp"
