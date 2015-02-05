#include <catch.hpp>
#include <rapidcheck-catch.h>

using namespace rc;

TEST_CASE("gen::character") {
    prop("never generates null characters", [] {
        RC_ASSERT(*gen::noShrink(gen::character<char>()) != '\0');
    });

    SECTION("does not shrink 'a'") {
        REQUIRE(!gen::character<char>().shrink('a')->hasNext());
    }

    prop("first tries to shrink every value to 'a')", [] {
        char c = *gen::character<char>();
        RC_PRE(c != 'a');
        RC_ASSERT(gen::character<char>().shrink(c)->next() == 'a');
    });
}
