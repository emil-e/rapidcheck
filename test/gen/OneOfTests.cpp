#include <catch.hpp>
#include <rapidcheck-catch.h>

using namespace rc;

TEST_CASE("gen::oneOf") {
    prop("only uses the given generators",
         [] (int a, int b, int c) {
             int value = *gen::noShrink(gen::oneOf(gen::constant(a),
                                                   gen::constant(b),
                                                   gen::constant(c)));
             RC_ASSERT((value == a) ||
                       (value == b) ||
                       (value == c));
         });

    prop("all generators are eventually used",
         [] (int a, int b, int c) {
             while (true) {
                 int value = *gen::noShrink(gen::oneOf(gen::constant(a),
                                                       gen::constant(b),
                                                       gen::constant(c)));
                 if (value == a)
                     break;
             }

             while (true) {
                 int value = *gen::noShrink(gen::oneOf(gen::constant(a),
                                                       gen::constant(b),
                                                       gen::constant(c)));
                 if (value == b)
                     break;
             }

             while (true) {
                 int value = *gen::noShrink(gen::oneOf(gen::constant(a),
                                                       gen::constant(b),
                                                       gen::constant(c)));
                 if (value == c)
                     break;
             }
         });
}
