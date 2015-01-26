#include <catch.hpp>
#include <rapidcheck-catch.h>

using namespace rc;

struct Showable {};

void show(Showable x, std::ostream &os) { os << "show(Showable)"; };

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

TEST_CASE("show<T>") {
    std::ostringstream os;

    SECTION("uses show(...) overload if available") {
        show(Showable(), os);
        REQUIRE(os.str() == "show(Showable)");
    }

    SECTION("tries to use operator<<(ostream...) if show(...) is not availble") {
        show(Ostreamable(), os);
        REQUIRE(os.str() == "<< Ostreamable");
    }

    SECTION("if neither operator<< or show(...) available, show as <\?\?\?>") {
        show(NonShowable(), os);
        REQUIRE(os.str() == "<\?\?\?>");
    }
}
