#pragma once

#include <string>

namespace rc {
namespace detail {

//! Throws a `Failure` result if `result` is false.
//!
//! @param description  A description of the potential failure.
//! @param result       `false` to throw, `true` to do nothing.
void assertThat(std::string description, bool result);

} // namespace detail
} // namespace rc
