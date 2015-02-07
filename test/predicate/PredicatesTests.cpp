#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/TypeListMacros.h"
#include "util/AppleOrange.h"
#include "util/Util.h"
#include "util/Meta.h"
#include "util/Predictable.h"

#include "rapidcheck/predicate/Predicates.h"

using namespace rc;
using namespace rc::predicate;

TEST_CASE("Not") {
    SECTION("returns the complement of the contained predicate") {
        const auto id = [](bool x) { return x; };
        const Not<decltype(id)> complement(id);
        REQUIRE(complement(true) == false);
        REQUIRE(complement(false) == true);
    }
}

#define DEFINE_BINARY_PREDICATE_TEST(Predicate, expr)                   \
    struct Test##Predicate                                              \
    {                                                                   \
        template<typename T>                                            \
        static void exec()                                              \
        {                                                               \
            templatedProp<T>(                                           \
                "implements " #expr,                                    \
                [] (const T value, const T &arg) {                      \
                    RC_ASSERT(Predicate<T>(value)(arg) == (expr));      \
                });                                                     \
        }                                                               \
    };                                                                  \
                                                                        \
    TEST_CASE(#Predicate) {                                             \
        meta::forEachType<Test##Predicate,                              \
                          RC_NUMERIC_TYPES,                             \
                          std::string,                                  \
                          const NonCopyable &>();                       \
                                                                        \
        prop("works when operands are of different type",               \
             [] (const Apple value, const Orange &arg) {                \
                 RC_ASSERT(Predicate<Apple>(value)(arg) == (expr));     \
             });                                                        \
    }

DEFINE_BINARY_PREDICATE_TEST(Equals, arg == value)
DEFINE_BINARY_PREDICATE_TEST(GreaterThan, arg > value)
DEFINE_BINARY_PREDICATE_TEST(LessThan, arg < value)
DEFINE_BINARY_PREDICATE_TEST(GreaterEqThan, arg >= value)
DEFINE_BINARY_PREDICATE_TEST(LessEqThan, arg <= value)

#undef DEFINE_BINARY_PREDICATE_TEST
