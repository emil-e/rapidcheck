#pragma once

#include "rapidcheck/Maybe.h"

namespace rc {
namespace detail {

/// Demangles a mangled C++ name.
std::string demangle(const char *name);

/// Returns the value of the environment variable with the given name or
/// `Nothing` if it is not set.
Maybe<std::string> getEnvValue(const std::string &name);

} // namespace detail
} // namespace rc
