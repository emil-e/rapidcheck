#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/detail/Configuration.h"

#include "util/TemplateProps.h"
#include "util/Generators.h"

using namespace rc;
using namespace rc::detail;

TEST_CASE("TestParams") {
  propConformsToEquals<TestParams>();
  propConformsToOutputOperator<TestParams>();
  PROP_REPLACE_MEMBER_INEQUAL(TestParams, seed);
  PROP_REPLACE_MEMBER_INEQUAL(TestParams, maxSuccess);
  PROP_REPLACE_MEMBER_INEQUAL(TestParams, maxSize);
  PROP_REPLACE_MEMBER_INEQUAL(TestParams, maxDiscardRatio);
}
