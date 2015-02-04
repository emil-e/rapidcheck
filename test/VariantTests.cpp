#include <catch.hpp>

#include "util/Util.h"

#include "rapidcheck/detail/Variant.h"

using namespace rc;
using namespace rc::detail;

struct A
{
    A() {}
    A(const std::string &x) : value(x) {}
    std::string value;
};

bool operator==(const A &a1, const A &a2)
{ return a1.value == a2.value; }

bool operator!=(const A &a1, const A &a2)
{ return a1.value != a2.value; }

struct B
{
    B() {}
    B(const std::string &x) : value(x) {}
    std::string value;
};

bool operator==(const B &b1, const B &b2)
{ return b1.value == b2.value; }

bool operator!=(const B &b1, const B &b2)
{ return b1.value != b2.value; }

struct C
{
    C() {}
    C(const std::string &x) : value(x) {}
    std::string value;
};

bool operator==(const C &c1, const C &c2)
{ return c1.value == c2.value; }

bool operator!=(const C &c1, const C &c2)
{ return c1.value != c2.value; }

typedef Variant<A, B, C> ABC;

struct Apple
{
    Apple(const char *x)
        : value(x) {}

    std::string value;
};

struct Orange
{
    Orange(const char *x)
        : value(x) {}

    std::string value;
};


inline bool operator==(const Apple &a1, const Apple &a2)
{ return a2.value == a2.value; }
inline bool operator==(const Orange &o1, const Orange &o2)
{ return o2.value == o2.value; }
inline bool operator!=(const Apple &a1, const Apple &a2) { return !(a1 == a2); }
inline bool operator!=(const Orange &o1, const Orange &o2) { return !(o1 == o2); }

// Apples and Oranges have comparison operators to compare each other
inline bool operator==(const Apple &a, const Orange &o)
{ return a.value == o.value; }
inline bool operator==(const Orange &o, const Apple &a) { return a == o; }
inline bool operator!=(const Apple &a, const Orange &o) { return !(a == o); }
inline bool operator!=(const Orange &o, const Apple &a) { return !(a == o); }


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

    SECTION("operator==/operator!=") {
        SECTION("equals if contained values are equal") {
            REQUIRE(ABC(A("a")) == ABC(A("a")));
            REQUIRE(ABC(B("b")) == ABC(B("b")));
            REQUIRE(ABC(C("c")) == ABC(C("c")));

            REQUIRE_FALSE(ABC(A("a")) != ABC(A("a")));
            REQUIRE_FALSE(ABC(B("b")) != ABC(B("b")));
            REQUIRE_FALSE(ABC(C("c")) != ABC(C("c")));
        }

        SECTION("not equals if contained values are not equal") {
            REQUIRE_FALSE(ABC(A("a")) == ABC(A("ax")));
            REQUIRE_FALSE(ABC(A("ax")) == ABC(A("a")));
            REQUIRE_FALSE(ABC(B("b")) == ABC(B("bx")));
            REQUIRE_FALSE(ABC(B("bx")) == ABC(B("b")));
            REQUIRE_FALSE(ABC(C("c")) == ABC(C("cx")));
            REQUIRE_FALSE(ABC(C("cx")) == ABC(C("c")));

            REQUIRE(ABC(A("a")) != ABC(A("ax")));
            REQUIRE(ABC(A("ax")) != ABC(A("a")));
            REQUIRE(ABC(B("b")) != ABC(B("bx")));
            REQUIRE(ABC(B("bx")) != ABC(B("b")));
            REQUIRE(ABC(C("c")) != ABC(C("cx")));
            REQUIRE(ABC(C("cx")) != ABC(C("c")));
        }

        SECTION("not equals if contained values are not of same type") {
            REQUIRE_FALSE(ABC(A("a")) == ABC(B("a")));
            REQUIRE_FALSE(ABC(B("a")) == ABC(A("a")));
            REQUIRE(ABC(A("a")) != ABC(B("a")));
            REQUIRE(ABC(B("a")) != ABC(A("a")));

            // Not even if types are normally equatable
            typedef Variant<Apple, Orange> Orapple;
            REQUIRE_FALSE(Orapple(Apple("foo")) == Orapple(Orange("foo")));
            REQUIRE_FALSE(Orapple(Orange("foo")) == Orapple(Apple("foo")));
            REQUIRE(Orapple(Apple("foo")) != Orapple(Orange("foo")));
            REQUIRE(Orapple(Orange("foo")) != Orapple(Apple("foo")));
        }
    }
}
