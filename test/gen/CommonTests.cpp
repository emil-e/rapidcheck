#include <catch.hpp>
#include <rapidcheck-catch.h>

using namespace rc;

TEST_CASE("gen::suchThat") {
    prop("never generates values not satisfying the predicate",
         [] (int max) {
             int x = *gen::noShrink(
                 gen::suchThat<int>([=](int x) { return x < max; }));
             RC_ASSERT(x < max);
         });
}

TEST_CASE("gen::constant") {
    prop("always returns the constant value",
         [] (int x) {
             auto generator = gen::constant(x);
             for (int i = 0; i < gen::currentSize(); i++)
                 RC_ASSERT(*generator == x);
         });
}

TEST_CASE("gen::map") {
    prop("maps a generated values from one type to another",
         [] (int input) {
             std::string str(
                 *gen::noShrink(
                     gen::map(gen::constant(input),
                              [] (int x) {
                                  return std::to_string(x);
                              })));
             RC_ASSERT(str == std::to_string(input));
         });
}
