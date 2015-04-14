#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/Seq.h"

#include "util/Generators.h"
#include "util/TemplateProps.h"
#include "util/Logger.h"

using namespace rc;
using namespace rc::test;

namespace {

class LoggingSeqImpl : public Logger
{
public:
    LoggingSeqImpl() : Logger() {}
    LoggingSeqImpl(std::string theId) : Logger(std::move(theId)) {}

    Maybe<std::pair<std::string, std::vector<std::string>>> operator()()
    { return {{ id, log }}; }
};

typedef Seq<std::pair<std::string, std::vector<std::string>>> LoggingSeq;

}

TEST_CASE("Seq") {
    SECTION("default constructed Seq is empty") {
        REQUIRE_FALSE(Seq<int>().next());
    }

    SECTION("calls operator()() of the implementation object") {
        bool nextCalled = false;
        Seq<int> seq = Seq<int>([&]{
            nextCalled = true;
            return Maybe<int>(1337);
        });

        REQUIRE(*seq.next() == 1337);
        REQUIRE(nextCalled);
    }

    SECTION("if exception is thrown on next(), Seq ends immediately") {
        auto x = 0;
        const auto seq = Seq<int>([x]() mutable -> Maybe<int> {
            if (x == 3)
                throw std::string("foobar");
            return ++x;
        });

        REQUIRE(seq == seq::just(1, 2, 3));
    }

    SECTION("operator==/operator!=") {
        propConformsToEquals<Seq<std::string>>();

        SECTION("empty sequences are equal") {
            REQUIRE(Seq<int>() == Seq<int>());
        }

        SECTION("an exhausted sequence equals an originally empty sequence") {
            auto seq = seq::just(1, 2, 3);
            seq.next();
            seq.next();
            seq.next();
            REQUIRE(seq == Seq<int>());
        }

        prop(
            "sequences with different implementation classes can be equal",
            [] (const std::string &a,
                const std::string &b,
                const std::string &c)
            {
                auto seqJust = seq::just(a, b, c);
                std::vector<std::string> vec{a, b, c};
                auto seqContainer = seq::fromContainer(vec);
                RC_ASSERT(seqJust == seqContainer);
            });

        prop(
            "changing a single element leads to inequal sequences",
            [] {
                // TODO non-empty generator
                const auto elements1 =
                    *gen::suchThat<std::vector<std::string>>(
                        [](const std::vector<std::string> &x) {
                            return !x.empty();
                        });
                auto elements2 = elements1;
                const auto i = *gen::inRange<std::size_t>(0, elements2.size());
                elements2[i] = *gen::distinctFrom(elements2[i]);
                RC_ASSERT(seq::fromContainer(elements1) !=
                          seq::fromContainer(elements2));
            });
    }

    SECTION("makeSeq") {
        SECTION("constructs implementation object in place") {
            auto seq = makeSeq<LoggingSeqImpl>("foobar");
            const auto value = seq.next();
            REQUIRE(value->first == "foobar");
            std::vector<std::string> expectedLog{"constructed as foobar"};
            REQUIRE(value->second == expectedLog);
        }
    }
}
