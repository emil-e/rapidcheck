#include "rapidcheck/detail/Utility.h"

#include <cxxabi.h>
#include <cstdlib>

namespace rc {
namespace detail {

std::string demangle(const char *name) {
  std::string demangled(name);
  int status;
  std::size_t length;
  char *buf = abi::__cxa_demangle(name, 0, 0, &status);
  if (status == 0) {
    demangled = std::string(buf);
  }
  free(buf);
  return demangled;
}

} // namespace detail
} // namespace rc
