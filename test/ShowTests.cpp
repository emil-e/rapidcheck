#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/Box.h"

using namespace rc;
using namespace rc::test;

namespace {

struct Showable {};

void showValue(Showable x, std::ostream &os) { os << "showValue(Showable)"; };

std::ostream &operator<<(std::ostream &os, Showable)
{
    os << "<< Showable";
    return os;
}

struct Ostreamable {};

std::ostream &operator<<(std::ostream &os, Ostreamable)
{
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

    SECTION("tries to use operator<<(ostream...) if showValue(...) is not availble") {
        REQUIRE(toString(Ostreamable()) == "<< Ostreamable");
    }

    SECTION("if neither operator<< or showValue(...) available, show as <\?\?\?>") {
        REQUIRE(toString(NonShowable()) == "<\?\?\?>");
    }
}

TEST_CASE("showCollection") {
    newprop(
        "shows empty collection correctly",
        [] (const std::string &prefix, const std::string &suffix) {
            std::ostringstream os;
            showCollection(prefix, suffix, std::vector<Box>(), os);
            RC_ASSERT(os.str() == (prefix + suffix));
        });

    newprop(
        "shows single element correctly",
        [&] (const std::string &prefix, const std::string &suffix, Box a) {
            std::ostringstream os;
            showCollection(prefix, suffix, std::vector<Box>{a}, os);
            RC_ASSERT(os.str() == (prefix + a.str() + suffix));
        });

    newprop(
        "shows multiple elements correctly",
        [&] (const std::string &prefix, const std::string &suffix,
             Box a, Box b, Box c)
        {
            std::ostringstream os;
            showCollection(prefix, suffix, std::vector<Box>{a, b, c}, os);
            RC_ASSERT(
                os.str() ==
                (prefix + a.str() + ", " + b.str() + ", " + c.str() + suffix));
        });
}
