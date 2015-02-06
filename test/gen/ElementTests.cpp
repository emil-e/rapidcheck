#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/Meta.h"
#include "util/Util.h"
#include "util/TypeListMacros.h"

using namespace rc;
using namespace rc::detail;

struct ElementOfTests
{
    template<typename T>
    static void exec()
    {
        static const auto nonEmpty = gen::suchThat<T>([] (const T &x) {
            return !x.empty();
        });

        templatedProp<T>(
            "all generated elements are elements of the container",
            [] {
                T elements = *nonEmpty;
                const auto value = *gen::elementOf(elements);
                RC_ASSERT(std::find(begin(elements), end(elements), value) !=
                          end(elements));
            });

        templatedProp<T>(
            "all elements are eventually generated",
            [] {
                T elements = *nonEmpty;
                const auto gen = gen::elementOf(elements);
                std::set<typename T::value_type> all(begin(elements), end(elements));
                std::set<typename T::value_type> generated;
                while (all != generated)
                    generated.insert(*gen);
                RC_SUCCEED("All values generated");
            });
    }
};

struct ResizableElementOfTests
{
    template<typename T>
    static void exec()
    {
        TEMPLATED_SECTION(T, "throws GenerationFailure on empty container") {
            T container;
            REQUIRE_THROWS_AS(*gen::elementOf(container),
                              gen::GenerationFailure);
        }
    }
};

TEST_CASE("gen::elementOf") {
    meta::forEachType<ElementOfTests,
                      RC_GENERIC_CONTAINERS(int),
                      RC_STRING_TYPES,
                      std::array<int, 10>>();
    meta::forEachType<ResizableElementOfTests,
                      RC_GENERIC_CONTAINERS(int),
                      RC_STRING_TYPES>();
}

TEST_CASE("gen::element") {
    prop("all generated elements are arguments",
         [] {
             const auto x = *gen::element<std::string>("foo", "bar", "baz");
             RC_ASSERT((x == "foo") || (x == "bar") || (x == "baz"));
         });

    prop("all generated elements are arguments",
         [] {
             const auto gen = gen::element<std::string>("foo", "bar", "baz");
             std::set<std::string> all{"foo", "bar", "baz"};
             std::set<std::string> generated;
             while (all != generated)
                 generated.insert(*gen);
             RC_SUCCEED("All values generated");
         });
}
