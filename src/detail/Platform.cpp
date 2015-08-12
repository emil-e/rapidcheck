#include "rapidcheck/detail/Platform.h"

#ifndef _MSC_VER
#include <cxxabi.h>
#endif // _MSC_VER

#include <cstdlib>

namespace rc {
namespace detail {

#ifdef _MSC_VER

std::string demangle(const char *name) {
  return name;
}

Maybe<std::string> getEnvValue(const std::string &name) {
  char *buffer = nullptr;
  const auto ret = _dupenv_s(&buffer, nullptr, name.c_str());
  const auto ptr = std::unique_ptr<char, decltype(&std::free)>(buffer, &std::free);

  if (buffer != nullptr) {
    return buffer;
  } else {
    return Nothing;
  }
}

#else // _MSC_VER

std::string demangle(const char *name) {
  std::string demangled(name);
  int status;
  char *buf = abi::__cxa_demangle(name, 0, 0, &status);
  if (status == 0) {
    demangled = std::string(buf);
  }
  free(buf);
  return demangled;
}

Maybe<std::string> getEnvValue(const std::string &name) {
  const auto value = std::getenv(name.c_str());
  if (value != nullptr) {
    return name;
  } else {
    return Nothing;
  }
}

#endif // _MSC_VER

} // namespace detail
} // namespace rc
