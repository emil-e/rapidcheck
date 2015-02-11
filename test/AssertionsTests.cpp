#include <catch.hpp>
#include <rapidcheck.h>

TEST_CASE("RC__CAPTURE") {
    REQUIRE(RC__CAPTURE(1 * 2 / 3 % 4) ==
            "1 * 2 / 3 % 4");
    REQUIRE(RC__CAPTURE(1 + 2 - 3) ==
            "1 + 2 - 3");
    REQUIRE(RC__CAPTURE(1 < 2 > 3 <= 4 >= 5) ==
            "1 < 2 > 3 <= 4 >= 5");
    REQUIRE(RC__CAPTURE(1 == 2 != 3) ==
            "1 == 2 != 3");
    REQUIRE(RC__CAPTURE(1 & 2 & 3) ==
            "1 & 2 & 3");
    REQUIRE(RC__CAPTURE(1 ^ 2 ^ 3) ==
            "1 ^ 2 ^ 3");
    REQUIRE(RC__CAPTURE(1 | 2 | 3) ==
            "1 | 2 | 3");
    REQUIRE(RC__CAPTURE(1 && 2 && 3) ==
            "1 && 2 && 3");
    REQUIRE(RC__CAPTURE(1 || 2 || 3) ==
            "1 || 2 || 3");

    std::cout << RC__CAPTURE(1 + 3 == 3 + 4);
}
