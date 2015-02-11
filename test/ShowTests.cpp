#include <catch.hpp>
#include <rapidcheck-catch.h>

using namespace rc;

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

TEST_CASE("show<T>") {
    std::ostringstream os;

    SECTION("uses showValue(...) overload if available") {
        show(Showable(), os);
        REQUIRE(os.str() == "showValue(Showable)");
    }

    SECTION("tries to use operator<<(ostream...) if showValue(...) is not availble") {
        show(Ostreamable(), os);
        REQUIRE(os.str() == "<< Ostreamable");
    }

    SECTION("if neither operator<< or showValue(...) available, show as <\?\?\?>") {
        show(NonShowable(), os);
        REQUIRE(os.str() == "<\?\?\?>");
    }
}
