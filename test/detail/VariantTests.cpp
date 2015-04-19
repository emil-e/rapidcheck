#include <catch.hpp>

#include "util/Util.h"
#include "util/Logger.h"
#include "util/AppleOrange.h"

#include "rapidcheck/detail/Variant.h"

using namespace rc;
using namespace rc::test;
using namespace rc::detail;

namespace {

template<std::size_t N>
struct X
{
    X() {}
    X(const std::string &x) : value(x) {}

    X(const X &other) noexcept
    { try { value = other.value; } catch(...) {} }

    X(X &&other) noexcept
    { try { value = std::move(other.value); } catch(...) {} }

    X &operator=(const X &other) noexcept
    {
        try { value = other.value; } catch(...) {}
        return *this;
    }

    X &operator=(X &&other) noexcept
    {
        try { value = std::move(other.value); } catch(...) {}
        return *this;
    }

    // Put some extra junk here so that different types have different layout.
    char extra[N];
    std::string value;
};

template<std::size_t N>
bool operator==(const X<N> &x1, const X<N> &x2)
{ return x1.value == x2.value; }

template<std::size_t N>
bool operator!=(const X<N> &x1, const X<N> &x2)
{ return x1.value != x2.value; }

typedef X<5> A;
typedef X<10> B;
typedef X<15> C;

typedef Variant<Logger, A, B, C> ABC;

} // namespace

TEST_CASE("Variant") {
    ABC va(A("A"));
    ABC vb(B("B"));
    ABC vc(C("C"));

    SECTION("universal constructor") {
        SECTION("rvalue") {
            ABC v(Logger("foobar"));
            REQUIRE(v.is<Logger>());
            REQUIRE(v.get<Logger>().id == "foobar");
            REQUIRE(v.get<Logger>().numberOf("copy") == 0);
        }

        SECTION("lvalue") {
            Logger logger("foobar");
            ABC v(logger);
            REQUIRE(v.is<Logger>());
            REQUIRE(v.get<Logger>().id == "foobar");
            REQUIRE(v.get<Logger>().numberOf("copy") == 1);
        }
    }

    SECTION("value assignment") {
        SECTION("from same type") {
            SECTION("rvalue") {
                ABC v(Logger("bar"));
                v = Logger("foo");
                REQUIRE(v.is<Logger>());
                REQUIRE(v.get<Logger>().id == "foo");
                REQUIRE(v.get<Logger>().numberOf("copy") == 0);
                REQUIRE(v.get<Logger>().numberOf("move") == 1);
                REQUIRE(v.get<Logger>().numberOf("move assigned") == 1);
            }

            SECTION("lvalue") {
                ABC v(Logger("bar"));
                Logger foo("foo");
                v = foo;
                REQUIRE(v.is<Logger>());
                REQUIRE(v.get<Logger>().id == "foo");
                REQUIRE(v.get<Logger>().numberOf("copy") == 1);
                REQUIRE(v.get<Logger>().numberOf("copy assigned") == 1);
            }
        }

        SECTION("from different type") {
            SECTION("rvalue") {
                ABC v(va);
                v = Logger("foo");
                REQUIRE(v.is<Logger>());
                REQUIRE(v.get<Logger>().id == "foo");
                REQUIRE(v.get<Logger>().numberOf("copy") == 0);
                REQUIRE(v.get<Logger>().numberOf("move") == 1);
                REQUIRE(v.get<Logger>().numberOf("move constructed") == 1);
            }

            SECTION("lvalue") {
                ABC v(va);
                Logger foo("foo");
                v = foo;
                REQUIRE(v.is<Logger>());
                REQUIRE(v.get<Logger>().id == "foo");
                REQUIRE(v.get<Logger>().numberOf("copy") == 1);
                REQUIRE(v.get<Logger>().numberOf("copy constructed") == 1);
            }
        }
    }

    SECTION("copy constructor") {
        ABC v1(Logger("foobar"));
        ABC v2(v1);
        REQUIRE(v2.is<Logger>());
        REQUIRE(v2.get<Logger>().id == "foobar");
        REQUIRE(v2.get<Logger>().numberOf("copy") == 1);
    }

    SECTION("move constructor") {
        ABC v1(Logger("foobar"));
        ABC v2(std::move(v1));
        REQUIRE(v2.is<Logger>());
        REQUIRE(v2.get<Logger>().id == "foobar");
        REQUIRE(v2.get<Logger>().numberOf("copy") == 0);
    }

    SECTION("copy assignment") {
        SECTION("from same type") {
            ABC v1(Logger("foo"));
            ABC v2(Logger("bar"));
            v2 = v1;
            REQUIRE(v2.is<Logger>());
            REQUIRE(v2.get<Logger>().id == "foo");
            REQUIRE(v2.get<Logger>().numberOf("copy") == 1);
            REQUIRE(v2.get<Logger>().numberOf("copy assigned") == 1);
        }

        SECTION("from different type") {
            ABC v1(Logger("foo"));
            ABC v2(va);
            v2 = v1;
            REQUIRE(v2.is<Logger>());
            REQUIRE(v2.get<Logger>().id == "foo");
            REQUIRE(v2.get<Logger>().numberOf("copy") == 1);
            REQUIRE(v2.get<Logger>().numberOf("copy constructed") == 1);
        }

        SECTION("self assignment leaves value unchanged") {
            ABC v(Logger("foo"));
            v = v;
            REQUIRE(v.is<Logger>());
            REQUIRE(v.get<Logger>().id == "foo");
        }
    }

    SECTION("move assignment") {
        SECTION("from same type") {
            ABC v1(Logger("foo"));
            ABC v2(Logger("bar"));
            v2 = std::move(v1);
            REQUIRE(v2.is<Logger>());
            REQUIRE(v2.get<Logger>().id == "foo");
            REQUIRE(v2.get<Logger>().numberOf("copy") == 0);
            REQUIRE(v2.get<Logger>().numberOf("move constructed") == 1);
            REQUIRE(v2.get<Logger>().numberOf("move assigned") == 1);
        }

        SECTION("from different type") {
            ABC v1(Logger("foo"));
            ABC v2(va);
            v2 = std::move(v1);
            REQUIRE(v2.is<Logger>());
            REQUIRE(v2.get<Logger>().id == "foo");
            REQUIRE(v2.get<Logger>().numberOf("copy") == 0);
            REQUIRE(v2.get<Logger>().numberOf("move constructed") == 2);
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
