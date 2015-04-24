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
    REQUIRE_THROWS_AS(
        configFromString("max_success=foobar"), ConfigurationException);
    REQUIRE_THROWS_AS(
        configFromString("max_success=-2"), ConfigurationException);
  }

  SECTION("throws on invalid maxSize") {
    REQUIRE_THROWS_AS(
        configFromString("max_size=foobar"), ConfigurationException);
    REQUIRE_THROWS_AS(configFromString("max_size=-2"), ConfigurationException);
  }

  SECTION("throws on invalid maxDiscardRatio") {
    REQUIRE_THROWS_AS(
        configFromString("max_discard_ratio=foobar"), ConfigurationException);
    REQUIRE_THROWS_AS(
        configFromString("max_discard_ratio=-2"), ConfigurationException);
  }

  SECTION("throws on invalid map format") {
    REQUIRE_THROWS_AS(
        configFromString("'max_discard_ratio=foobar"), ConfigurationException);
    REQUIRE_THROWS_AS(
        configFromString("max_discard_ratio=\"-2"), ConfigurationException);
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
