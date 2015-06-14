#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/Shrinkable.h"
#include "rapidcheck/shrinkable/Create.h"

#include "util/TemplateProps.h"
#include "util/Logger.h"
#include "util/Generators.h"
#include "util/DestructNotifier.h"

using namespace rc;
using namespace rc::test;

namespace {

template <typename ValueCallable, typename ShrinksCallable>
class MockShrinkableImpl {
public:
  MockShrinkableImpl(ValueCallable value, ShrinksCallable shrinks)
      : m_value(value)
      , m_shrinks(shrinks) {}

  typename std::result_of<ValueCallable()>::type value() const {
    return m_value();
  }

  typename std::result_of<ShrinksCallable()>::type shrinks() const {
    return m_shrinks();
  }

private:
  ValueCallable m_value;
  ShrinksCallable m_shrinks;
};

template <typename ValueCallable, typename ShrinksCallable>
Shrinkable<Decay<typename std::result_of<ValueCallable()>::type>>
makeMockShrinkable(ValueCallable value, ShrinksCallable shrinks) {
  return makeShrinkable<MockShrinkableImpl<ValueCallable, ShrinksCallable>>(
      value, shrinks);
}

class LoggingShrinkableImpl : public Logger {
public:
  using IdLogPair = std::pair<std::string, std::vector<std::string>>;

  LoggingShrinkableImpl()
      : Logger() {}
  LoggingShrinkableImpl(std::string theId)
      : Logger(std::move(theId)) {}

  IdLogPair value() const { return {id, log}; }

  Seq<Shrinkable<IdLogPair>> shrinks() const {
    return Seq<Shrinkable<IdLogPair>>();
  }
};

using LoggingShrinkable =
    Shrinkable<std::pair<std::string, std::vector<std::string>>>;

} // namespace

TEST_CASE("Shrinkable") {
  SECTION("calls value() of the implementation object") {
    bool valueCalled = false;
    Shrinkable<int> shrinkable = makeMockShrinkable(
        [&] {
          valueCalled = true;
          return 1337;
        },
        [] { return Seq<Shrinkable<int>>(); });

    REQUIRE(shrinkable.value() == 1337);
    REQUIRE(valueCalled);
  }

  SECTION("calls shrinks() of the implementation object") {
    Shrinkable<int> shrink = makeMockShrinkable(
        [] { return 123; }, [] { return Seq<Shrinkable<int>>(); });
    auto shrinks = seq::just(shrink);

    bool shrinksCalled = false;
    Shrinkable<int> shrinkable = makeMockShrinkable([] { return 0; },
                                                    [&] {
                                                      shrinksCalled = true;
                                                      return shrinks;
                                                    });

    REQUIRE(shrinkable.shrinks() == shrinks);
    REQUIRE(shrinksCalled);
  }

  SECTION("self assignment leaves value unchanged") {
    const auto shrinkable =
        shrinkable::just(13, seq::just(shrinkable::just(37)));
    auto x = shrinkable;
    x = x;
    REQUIRE(x == shrinkable);
  }

  SECTION("if shrinks() throws, an empty Seq is returned") {
    Shrinkable<int> shrinkable = makeMockShrinkable(
        [] { return 0; },
        []() -> Seq<Shrinkable<int>> { throw std::string("foobar"); });

    REQUIRE(!shrinkable.shrinks().next());
  }

  SECTION("retains implementation object until no copies remain") {
    std::vector<std::string> log;
    Maybe<Shrinkable<DestructNotifier>> s1 =
        shrinkable::just(DestructNotifier("foobar", &log));
    REQUIRE(log.empty());

    Maybe<Shrinkable<DestructNotifier>> s2 = s1;
    REQUIRE(log.empty());

    s1.reset();
    REQUIRE(log.empty());

    s2.reset();
    REQUIRE(log.size() == 1);
    REQUIRE(log[0] == "foobar");
  }

  SECTION("moving steals reference") {
    std::vector<std::string> log;
    auto s1 = shrinkable::just(DestructNotifier("foobar", &log));

    {
      const auto s2 = std::move(s1);
      REQUIRE(log.empty());
    }

    REQUIRE(log.size() == 1);
    REQUIRE(log[0] == "foobar");
  }

  SECTION("operator==/operator!=") {
    propConformsToEquals<Shrinkable<int>>();

    prop("different values yield inequal shrinkables",
         [](Seq<Shrinkable<int>> shrinks, int v1) {
           int v2 = *gen::distinctFrom(v1);
           RC_ASSERT(shrinkable::just(v1, shrinks) !=
                     shrinkable::just(v2, shrinks));
         });

    prop("different shrinks yield inequal shrinkables",
         [](int value, Seq<Shrinkable<int>> shrinks1) {
           Seq<Shrinkable<int>> shrinks2 = *gen::distinctFrom(shrinks1);
           RC_ASSERT(shrinkable::just(value, shrinks1) !=
                     shrinkable::just(value, shrinks2));
         });
  }

  SECTION("makeShrinkable") {
    SECTION("constructs implementation object in place") {
      auto shrinkable = makeShrinkable<LoggingShrinkableImpl>("foobar");
      const auto value = shrinkable.value();
      REQUIRE(value.first == "foobar");
      std::vector<std::string> expectedLog{"constructed as foobar"};
      REQUIRE(value.second == expectedLog);
    }
  }
}
