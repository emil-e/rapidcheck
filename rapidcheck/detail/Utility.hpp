#pragma once

#include <cxxabi.h>
#include <cstdlib>

namespace rc {
namespace detail {

//! Disables copying
#define RC_DISABLE_COPY(Class)                    \
    Class(const Class &) = delete;                \
    Class &operator=(const Class &) = delete;

//! Disables moving
#define RC_DISABLE_MOVE(Class)            \
    Class(Class &&) = delete;             \
    Class &operator=(Class &&) = delete;

std::string demangle(const char *name)
{
    std::string demangled(name);
    int status;
    size_t length;
    char *buf = abi::__cxa_demangle(name, NULL, &length, &status);
    if (status == 0)
        demangled = std::string(buf, length);
    free(buf);
    return demangled;
}

} // namespace detail
} // namespace rc
