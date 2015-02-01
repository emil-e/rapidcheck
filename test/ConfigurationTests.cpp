#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/TemplateProps.h"
#include "util/Generators.h"

#include "rapidcheck/detail/Configuration.h"

using namespace rc;
using namespace rc::detail;

TEST_CASE("Configuration") {
    SECTION("operator==/operator!=") {
        propConformsToEquals<Configuration>();
        PROP_REPLACE_MEMBER_INEQUAL(Configuration, seed);
        PROP_REPLACE_MEMBER_INEQUAL(Configuration, maxSuccess);
        PROP_REPLACE_MEMBER_INEQUAL(Configuration, maxSize);
        PROP_REPLACE_MEMBER_INEQUAL(Configuration, maxDiscardRatio);
    }
}

TEST_CASE("configFromString") {
    SECTION("emtpy string yields default config") {
        REQUIRE(configFromString("") == Configuration());
    }

    SECTION("ignores unknown keys") {
        REQUIRE(configFromString("foo=bar stuff=things") == Configuration());
    }

    SECTION("throws on invalid seed") {
        REQUIRE_THROWS_AS(configFromString("seed=foobar"),
                          ConfigurationException);
        REQUIRE_THROWS_AS(configFromString("seed=-2"),
                          ConfigurationException);
    }

    SECTION("throws on invalid maxSuccess") {
        REQUIRE_THROWS_AS(configFromString("max_success=foobar"),
                          ConfigurationException);
        REQUIRE_THROWS_AS(configFromString("max_success=-2"),
                          ConfigurationException);
    }

    SECTION("throws on invalid maxSize") {
        REQUIRE_THROWS_AS(configFromString("max_size=foobar"),
                          ConfigurationException);
        REQUIRE_THROWS_AS(configFromString("max_size=-2"),
                          ConfigurationException);
    }

    SECTION("throws on invalid maxDiscardRatio") {
        REQUIRE_THROWS_AS(configFromString("max_discard_ratio=foobar"),
                          ConfigurationException);
        REQUIRE_THROWS_AS(configFromString("max_discard_ratio=-2"),
                          ConfigurationException);
    }

    SECTION("throws on invalid map format") {
        REQUIRE_THROWS_AS(configFromString("'max_discard_ratio=foobar"),
                          ConfigurationException);
        REQUIRE_THROWS_AS(configFromString("max_discard_ratio=\"-2"),
                          ConfigurationException);
    }

    prop("configFromString(configToString(x)) == x",
         [] (const Configuration &config) {
             RC_ASSERT(configFromString(configToString(config)) == config);
         });
}


TEST_CASE("configToMinimalString") {
    SECTION("default configuration yields empty string") {
        REQUIRE(configToMinimalString(Configuration()) == "");
    }

    prop("configFromString(configToMinimalString(x)) == x",
         [] (const Configuration &config) {
             RC_ASSERT(configFromString(configToMinimalString(config)) ==
                       config);
         });
}
