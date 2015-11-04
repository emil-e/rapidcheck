#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "util/Box.h"

using namespace rc;
using namespace rc::test;

namespace {

struct Showable {};

void showValue(Showable x, std::ostream &os) { os << "showValue(Showable)"; };

// This is intentonally not used, that's part of the test. We ensure below that
// showValue is selected over operator<<
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunneeded-internal-declaration"
#endif // __clang__

std::ostream &operator<<(std::ostream &os, Showable) {
  os << "<< Showable";
  return os;
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__

struct Ostreamable {};

std::ostream &operator<<(std::ostream &os, Ostreamable) {
  os << "<< Ostreamable";
  return os;
}

struct NonShowable {};

} // namespace

TEST_CASE("toString<T>") {
  std::ostringstream os;

  SECTION("uses showValue(...) overload if available") {
    REQUIRE(toString(Showable()) == "showValue(Showable)");
  }

  SECTION(
      "tries to use operator<<(ostream...) if showValue(...) is not availble") {
    REQUIRE(toString(Ostreamable()) == "<< Ostreamable");
  }

  SECTION(
      "if neither operator<< or showValue(...) available, show as <\?\?\?>") {
    REQUIRE(toString(NonShowable()) == "<\?\?\?>");
  }
}

TEST_CASE("showCollection") {
  prop("shows empty collection correctly",
       [](const std::string &prefix, const std::string &suffix) {
         std::ostringstream os;
         showCollection(prefix, suffix, std::vector<Box>(), os);
         RC_ASSERT(os.str() == (prefix + suffix));
       });

  prop("shows single element correctly",
       [&](const std::string &prefix, const std::string &suffix, Box a) {
         std::ostringstream os;
         showCollection(prefix, suffix, std::vector<Box>{a}, os);
         RC_ASSERT(os.str() == (prefix + a.str() + suffix));
       });

  prop("shows multiple elements correctly",
       [&](const std::string &prefix,
           const std::string &suffix,
           Box a,
           Box b,
           Box c) {
         std::ostringstream os;
         showCollection(prefix, suffix, std::vector<Box>{a, b, c}, os);
         RC_ASSERT(os.str() == (prefix + a.str() + ", " + b.str() + ", " +
                                c.str() + suffix));
       });
}
