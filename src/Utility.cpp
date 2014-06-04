#include "rapidcheck/detail/Utility.h"

#include <cxxabi.h>
#include <cstdlib>

namespace rc {
namespace detail {

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
