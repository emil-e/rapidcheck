#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/detail/TestMetadata.h"

#include "util/TemplateProps.h"
#include "util/Generators.h"

using namespace rc;
using namespace rc::detail;

TEST_CASE("TestMetadata") {
  SECTION("operator==/operator!=") {
    propConformsToEquals<TestMetadata>();
    PROP_REPLACE_MEMBER_INEQUAL(TestMetadata, id);
    PROP_REPLACE_MEMBER_INEQUAL(TestMetadata, description);
  }

  SECTION("operator<<") { propConformsToOutputOperator<TestMetadata>(); }
}
