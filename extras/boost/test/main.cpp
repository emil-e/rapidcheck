#include <exception>
#if (__GLIBCXX__ / 10000) == 2014

namespace std {

inline bool uncaught_exception() noexcept(true) {
  return current_exception() != nullptr;
}

} // namespace std

#endif

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
