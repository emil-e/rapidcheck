#include <catch.hpp>
#include <rapidcheck-catch.h>

using namespace rc::detail;

using StorageT = PolymorphicStorage<sizeof(void *) * 4>;

namespace {

template<std::size_t N>
struct PolyMock : public IPolymorphic {
    explicit PolyMock(std::string *s) : status(s)
    { *status = "constructed " + std::to_string(N); }

    void copyInto(void *storage) const override
    {
        *status = "copied";
        new (storage) PolyMock(*this);
    }

    void moveInto(void *storage) override
    {
        *status = "moved";
        new (storage) PolyMock(*this);
    }

    ~PolyMock() { *status = "destroyed"; }

    std::string *status;
    static constexpr auto extraSize = N;
    uint8_t extra[N];
};

using TinyPoly = PolyMock<0>;
using SmallPoly = PolyMock<StorageT::size - sizeof(PolyMock<0>)>;
using LargePoly = PolyMock<SmallPoly::extraSize + 1>;

} // namespace

TEST_CASE("PolymorphicStorage") {
    std::string log;
    std::string logAlt;

    SECTION("initWithFallback") {
        SECTION("uses preferred type if small enough") {
            StorageT storage;
            storage.initWithFallback<SmallPoly, TinyPoly>(&log);
            REQUIRE(
                log == "constructed " + std::to_string(SmallPoly::extraSize));
        }

        SECTION("uses fallback type if not small enough") {
            StorageT storage;
            storage.initWithFallback<LargePoly, TinyPoly>(&log);
            REQUIRE(
                log == "constructed " + std::to_string(TinyPoly::extraSize));
        }
    }

    SECTION("copy constructor") {
        StorageT a;
        a.init<TinyPoly>(&log);
        StorageT b(a);

        REQUIRE(log == "copied");
        REQUIRE(a.get<TinyPoly>().status == b.get<TinyPoly>().status);
    }

    SECTION("move constructor") {
        StorageT a;
        a.init<TinyPoly>(&log);
        StorageT b(std::move(a));

        REQUIRE(log == "moved");
        REQUIRE(a.get<TinyPoly>().status == b.get<TinyPoly>().status);
    }

    SECTION("copy assignment") {
        StorageT a;
        a.init<TinyPoly>(&log);
        StorageT b;
        b.init<TinyPoly>(&logAlt);
        b = a;

        REQUIRE(log == "copied");
        REQUIRE(a.get<TinyPoly>().status == b.get<TinyPoly>().status);
    }

    SECTION("move assignment") {
        StorageT a;
        a.init<TinyPoly>(&log);
        StorageT b;
        b.init<TinyPoly>(&logAlt);
        b = std::move(a);

        REQUIRE(log == "moved");
        REQUIRE(a.get<TinyPoly>().status == b.get<TinyPoly>().status);
    }

    SECTION("destructor") {
        {
            StorageT storage;
            storage.init<TinyPoly>(&log);
        }
        REQUIRE(log == "destroyed");
    }
}
