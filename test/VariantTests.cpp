#include <catch.hpp>

#include "rapidcheck/detail/Variant.h"

using namespace rc;
using namespace rc::detail;

struct A
{
    A() {}
    A(const std::string &x) : value(x) {}
    std::string value;
};

struct B
{
    B() {}
    B(const std::string &x) : value(x) {}
    std::string value;
};

struct C
{
    C() {}
    C(const std::string &x) : value(x) {}
    std::string value;
};

typedef Variant<A, B, C> ABC;

TEST_CASE("Variant") {
    ABC va(A("A"));
    ABC vb(B("B"));
    ABC vc(C("C"));

    SECTION("constructor") {
        SECTION("works rvalue references") {
            A a("foobar");
            A ap;
            ABC v1(static_cast<A &&>(a));
            REQUIRE(v1.match(ap));
            REQUIRE(ap.value == "foobar");
        }

        SECTION("works lvalue references") {
            A a("foobar");
            A ap;
            ABC v1(static_cast<A &>(a));
            REQUIRE(v1.match(ap));
            REQUIRE(ap.value == "foobar");
        }
    }

    SECTION("is") {
        SECTION("returns true if the value has the given type") {
            REQUIRE(va.is<A>());
            REQUIRE(vb.is<B>());
            REQUIRE(vc.is<C>());
        }

        SECTION("returns false if value does not have the given type") {
            REQUIRE(!va.is<B>());
            REQUIRE(!va.is<C>());

            REQUIRE(!vb.is<A>());
            REQUIRE(!vb.is<C>());

            REQUIRE(!vc.is<A>());
            REQUIRE(!vc.is<B>());
        }
    }

    SECTION("match") {
        A a("AAA");
        B b("BBB");
        C c("CCC");

        SECTION("returns true if the value has the given type") {
            REQUIRE(va.match(a));
            REQUIRE(vb.match(b));
            REQUIRE(vc.match(c));
        }

        SECTION("returns false if value does not have the given type") {
            REQUIRE(!va.match(b));
            REQUIRE(!va.match(c));

            REQUIRE(!vb.match(a));
            REQUIRE(!vb.match(c));

            REQUIRE(!vc.match(a));
            REQUIRE(!vc.match(b));
        }

        SECTION("leaves given value untouched if no match") {
            A aa(a);
            vb.match(aa);
            vc.match(aa);
            REQUIRE(a.value == aa.value);

            B bb(b);
            va.match(bb);
            vc.match(bb);
            REQUIRE(b.value == bb.value);

            C cc(c);
            va.match(cc);
            vb.match(cc);
            REQUIRE(c.value == cc.value);
        }

        SECTION("sets the value to the contained value on match") {
            va.match(a);
            REQUIRE(a.value == "A");
            vb.match(b);
            REQUIRE(b.value == "B");
            vc.match(c);
            REQUIRE(c.value == "C");
        }
    }
}
