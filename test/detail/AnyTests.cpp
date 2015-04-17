#include <catch.hpp>

#include "rapidcheck/detail/Any.h"

#include "util/Util.h"
#include "util/Predictable.h"

using namespace rc;
using namespace rc::detail;

namespace {

enum class InitType { Value, Move, Copy, Dead };

std::ostream &operator<<(std::ostream &os, InitType itype)
{
    switch (itype) {
    case InitType::Value:
        os << "Value";
        break;
    case InitType::Move:
        os << "Moved";
        break;
    case InitType::Copy:
        os << "Copied";
        break;
    case InitType::Dead:
        os << "Dead";
        break;
    }

    return os;
}

template<typename T>
struct InitTracker
{
    InitType itype;
    T value;

    InitTracker(T v)
        : itype(InitType::Value)
        , value(v) {}
    InitTracker(const InitTracker<T> &other)
        : itype(InitType::Copy)
        , value(other.value) {}
    InitTracker(InitTracker<T> &&other)
        : itype(InitType::Move)
        , value(std::move(other.value))
    { other.itype = InitType::Dead; }

    InitTracker<T> &operator=(InitTracker<T> &&rhs)
    {
        itype = InitType::Move;
        value = std::move(rhs.value);
        rhs.itype = InitType::Dead;
    }

    InitTracker<T> &operator=(const InitTracker<T> &rhs)
    {
        itype = InitType::Copy;
        value = std::move(rhs.value);
    }
};

template<typename T>
void show(const InitTracker<T> &value, std::ostream &os)
{
    os << "[" << value.itype << "]";
    if (value.itype != InitType::Dead) {
        os << ": ";
        rc::show(value.value, os);
    }
}

typedef InitTracker<std::string> StringTracker;

} // namespace

TEST_CASE("Any") {
    StringTracker x("foobar");
    REQUIRE(x.itype == InitType::Value);

    SECTION("of") {
        SECTION("allows move") {
            Any any = Any::of(std::move(x));
            auto &valueRef = any.get<StringTracker>();
            REQUIRE(valueRef.itype == InitType::Move);
        }

        SECTION("allows copy") {
            Any any = Any::of(x);
            auto &valueRef = any.get<StringTracker>();
            REQUIRE(valueRef.itype == InitType::Copy);
        }
    }

    SECTION("reset") {
        SECTION("turns this into null any") {
            Any any = Any::of("foobar");
            any.reset();
            REQUIRE(!any);
        }
    }

    SECTION("get") {
        SECTION("returns a reference to the value") {
            Any anystr = Any::of(std::string("foobar"));
            REQUIRE(anystr.get<std::string>() == "foobar");
            anystr.get<std::string>()[0] = 'b';
            REQUIRE(anystr.get<std::string>() == "boobar");

            Any anyint = Any::of(static_cast<int>(100));
            REQUIRE(anyint.get<int>() == 100);
            anyint.get<int>()++;
            REQUIRE(anyint.get<int>() == 101);
        }

        SECTION("works as a move source") {
            std::string str("foobar");
            Any any = Any::of(str);
            std::string dest(std::move(any.get<std::string>()));
            REQUIRE(str == dest);
        }
    }

    SECTION("describe") {
        SECTION("returns a pair of the typename and value as strings") {
            std::string str("foobar");
            std::pair<std::string, std::string> desc(typeToString<decltype(str)>(),
                                                     toString(str));

            Any any = Any::of(str);
            auto anyDesc(any.describe());

            REQUIRE(desc == anyDesc);
        }
    }

    SECTION("operator bool()") {
        SECTION("returns false for null Any") {
            Any any;
            REQUIRE(!any);
        }

        SECTION("returns true for non-null Any") {
            Any any = Any::of(1337);
            REQUIRE(any);
        }
    }

    SECTION("move constructor") {
        SECTION("if not null") {
            std::string str("foobar");
            Any from = Any::of(str);
            auto addr = &from.get<std::string>();
            Any to(std::move(from));

            SECTION("value is equal to original") {
                REQUIRE(to.get<std::string>() == str);
            }
            SECTION("original is reset") {
                REQUIRE(!from);
            }
            SECTION("addr is the same as original") {
                REQUIRE(&to.get<std::string>() == addr);
            }
        }

        SECTION("if from is null, to is also null") {
            Any from;
            Any to(std::move(from));
            REQUIRE(!to);
        }
    }

    SECTION("move assignment operator") {
        SECTION("if not null") {
            std::string str("foobar");
            Any from = Any::of(str);
            auto addr = &from.get<std::string>();
            Any to;
            to = std::move(from);

            SECTION("value is equal to original") {
                REQUIRE(to.get<std::string>() == str);
            }
            SECTION("original is reset") {
                REQUIRE(!from);
            }
            SECTION("addr is the same as original") {
                REQUIRE(&to.get<std::string>() == addr);
            }
        }

        SECTION("if from is null, to is also null") {
            Any from;
            Any to;
            to = std::move(from);
            REQUIRE(!to);
        }
    }
}
