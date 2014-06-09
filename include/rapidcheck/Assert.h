#pragma once

#include "detail/Assert.h"

namespace rc {

//! Asserts that the given expression is true.
#define ASSERT_THAT(expr) ::rc::detail::assertThat(#expr, (expr))

} // namespace rc
