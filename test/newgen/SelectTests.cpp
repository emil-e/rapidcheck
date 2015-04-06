#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/newgen/Create.h"
#include "rapidcheck/newgen/Select.h"

#include "util/Meta.h"
#include "util/Util.h"
#include "util/TypeListMacros.h"
#include "util/GenUtils.h"
#include "util/Predictable.h"

using namespace rc;
using namespace rc::test;
using namespace rc::detail;

struct ElementOfTests
{
    template<typename T>
    static void exec()
    {
        // TODO should be in standard library of generators!
        static const auto nonEmpty = newgen::suchThat<T>([](const T &x) {
            return !x.empty();
        });

        newtemplatedProp<T>(
            "all generated elements are elements of the container",
            [](const GenParams &params) {
                T elements = *nonEmpty;
                const auto gen = newgen::elementOf(elements);
                const auto value = gen(params.random, params.size).value();
                RC_ASSERT(std::find(begin(elements), end(elements), value) !=
                          end(elements));
            });

        newtemplatedProp<T>(
            "all elements are eventually generated",
            [](const GenParams &params) {
                T elements = *nonEmpty;
                const auto gen = newgen::elementOf(elements);
                std::set<typename T::value_type> all(begin(elements),
                                                     end(elements));
                std::set<typename T::value_type> generated;
                Random r(params.random);
                while (all != generated)
                    generated.insert(gen(r.split(), params.size).value());
                RC_SUCCEED("All values generated");
            });

        TEMPLATED_SECTION(T, "throws GenerationFailure on empty container") {
            T container;
            const auto shrinkable = newgen::elementOf(container)(Random(0), 0);
            REQUIRE_THROWS_AS(shrinkable.value(), GenerationFailure);
        }
    }
};

TEST_CASE("newgen::elementOf") {
    meta::forEachType<ElementOfTests,
                      std::vector<char>,
                      std::string>();
}

TEST_CASE("newgen::element") {
    newprop(
        "all generated elements are arguments",
        [](const GenParams &params,
           const std::string &a,
           const std::string &b,
           const std::string &c) {
            const auto gen = newgen::element(a, b, c);
            const auto x = gen(params.random, params.size).value();
            RC_ASSERT((x == a) || (x == b) || (x == c));
        });

    newprop(
        "all elements are eventually generated",
        [](const GenParams &params,
           const std::string &a,
           const std::string &b,
           const std::string &c) {
            const auto gen = newgen::element(a, b, c);
            std::set<std::string> all{a, b, c};
            std::set<std::string> generated;
            Random r(params.random);
            while (all != generated)
                generated.insert(gen(r.split(), params.size).value());
            RC_SUCCEED("All values generated");
        });
}

TEST_CASE("newgen::oneOf") {
    newprop(
        "all generated elements come from one of the generators",
        [](const GenParams &params,
           const std::string &a,
           const std::string &b,
           const std::string &c) {
            const auto gen = newgen::oneOf(newgen::just(a),
                                           newgen::just(b),
                                           newgen::just(c));
            const auto x = gen(params.random, params.size).value();
            RC_ASSERT((x == a) || (x == b) || (x == c));
        });

    newprop(
        "all generators are eventually used",
        [](const GenParams &params,
           const std::string &a,
           const std::string &b,
           const std::string &c) {
            const auto gen = newgen::oneOf(newgen::just(a),
                                           newgen::just(b),
                                           newgen::just(c));
            std::set<std::string> all{a, b, c};
            std::set<std::string> generated;
            Random r(params.random);
            while (all != generated)
                generated.insert(gen(r.split(), params.size).value());
            RC_SUCCEED("All values generated");
        });

    newprop(
        "passes the correct size to the generators",
        [](const GenParams &params) {
            const auto gen = newgen::oneOf(genPassedParams());
            const auto value = gen(params.random, params.size).value();
            RC_ASSERT(value.size == params.size);
        });

    newprop(
        "works with non-copyable types",
        [](const GenParams &params) {
            const auto gen = newgen::oneOf(newgen::arbitrary<NonCopyable>());
            const auto value = gen(params.random, params.size).value();
            RC_ASSERT(isArbitraryPredictable(value));
        });
}
