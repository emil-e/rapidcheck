#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>

#include "util/TemplateProps.h"
#include "util/Generators.h"

#include "rapidcheck/detail/Configuration.h"

using namespace rc;
using namespace rc::detail;

TEST_CASE("Configuration") {
  SECTION("operator==/operator!=") {
    propConformsToEquals<Configuration>();
    PROP_REPLACE_MEMBER_INEQUAL(Configuration, testParams);
    PROP_REPLACE_MEMBER_INEQUAL(Configuration, verboseProgress);
    PROP_REPLACE_MEMBER_INEQUAL(Configuration, verboseShrinking);
  }

  SECTION("operator<<") { propConformsToOutputOperator<Configuration>(); }
}

TEST_CASE("configFromString") {
  prop("emtpy string yields default config",
       [](const Configuration &config) {
         RC_ASSERT(configFromString("", config) == config);
       });

  prop("ignores unknown keys",
       [](const Configuration &config) {
         RC_ASSERT(configFromString("foo=bar stuff=things", config) == config);
       });

  SECTION("throws on invalid seed") {
    REQUIRE_THROWS_AS(configFromString("seed=foobar"), ConfigurationException);
    REQUIRE_THROWS_AS(configFromString("seed=--2"), ConfigurationException);
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
    REQUIRE_THROWS_AS(configFromString("max_size=-2"), ConfigurationException);
  }

  SECTION("throws on invalid maxDiscardRatio") {
    REQUIRE_THROWS_AS(configFromString("max_discard_ratio=foobar"),
                      ConfigurationException);
    REQUIRE_THROWS_AS(configFromString("max_discard_ratio=-2"),
                      ConfigurationException);
  }

  SECTION("throws on invalid noshrink setting") {
    REQUIRE_THROWS_AS(configFromString("noshrink=foobar"),
                      ConfigurationException);
    REQUIRE_THROWS_AS(configFromString("noshrink=2"), ConfigurationException);
  }

  SECTION("throws on invalid verbose progress setting") {
    REQUIRE_THROWS_AS(configFromString("verbose_progress=foo"),
                      ConfigurationException);
    REQUIRE_THROWS_AS(configFromString("verbose_progress=2"),
                      ConfigurationException);
  }

  SECTION("throws on invalid verbose shrinking setting") {
    REQUIRE_THROWS_AS(configFromString("verbose_shrinking=foo"),
                      ConfigurationException);
    REQUIRE_THROWS_AS(configFromString("verbose_shrinking=2"),
                      ConfigurationException);
  }

  SECTION("throws on invalid map format") {
    REQUIRE_THROWS_AS(configFromString("'max_discard_ratio=foobar"),
                      ConfigurationException);
    REQUIRE_THROWS_AS(configFromString("max_discard_ratio=\"-2"),
                      ConfigurationException);
  }

  prop("configFromString(configToString(x)) == x",
       [](const Configuration &config) {
         RC_ASSERT(configFromString(configToString(config)) == config);
       });
}

TEST_CASE("configToMinimalString") {
  prop("is always shorter or same size as configFromString",
       [](const Configuration &config) {
         RC_ASSERT(configToMinimalString(config).size() <=
                   configToString(config).size());
       });

  prop("configFromString(configToMinimalString(x)) == x",
       [](const Configuration &config) {
         RC_ASSERT(configFromString(configToMinimalString(config)) == config);
       });
}
