#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/detail/ApplyTuple.h"

#include "util/Logger.h"

using namespace rc;
using namespace rc::test;
using namespace rc::detail;

std::tuple<int, std::string, std::vector<std::string>>
    myFunc(int x, int y, Logger logger)
{
    return {x + y, logger.id, logger.log};
}

TEST_CASE("applyTuple") {
    auto tuple = std::make_tuple(40, 2, Logger("foobar"));
    std::tuple<int, std::string, std::vector<std::string>> result;
    std::vector<std::string> expectedLog;

    SECTION("std::tuple<...> &&") {
        result = applyTuple(std::move(tuple), &myFunc);
        expectedLog = {
            "constructed as foobar",
            "move constructed",
            "move constructed"
        };
    }

    SECTION("std::tuple<...> &") {
        result = applyTuple(tuple, &myFunc);
        expectedLog = {
            "constructed as foobar",
            "move constructed",
            "copy constructed"
        };
    }

    SECTION("const std::tuple<...> &") {
        const auto &constTuple = tuple;
        result = applyTuple(constTuple, &myFunc);
        expectedLog = {
            "constructed as foobar",
            "move constructed",
            "copy constructed"
        };
    }

    REQUIRE(std::get<0>(result) == 42);
    REQUIRE(std::get<1>(result) == "foobar");
    REQUIRE(std::get<2>(result) == expectedLog);
}
