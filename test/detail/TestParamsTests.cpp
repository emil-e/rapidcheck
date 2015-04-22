#include <catch.hpp>
#include <rapidcheck-catch.h>

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

TEST_CASE("defaultTestParams") {
    prop(
        "takes params from current configuration",
        [](uint64_t seed, int maxSuccess, int maxSize, int maxDiscardRatio) {
            Configuration config;
            config.seed = seed;
            config.maxSuccess = maxSuccess;
            config.maxSize = maxSize;
            config.maxDiscardRatio = maxDiscardRatio;
            ImplicitParam<param::CurrentConfiguration> letConfig(config);
            TestParams params = defaultTestParams();
            RC_ASSERT(params.seed == seed);
            RC_ASSERT(params.maxSuccess == maxSuccess);
            RC_ASSERT(params.maxSize == maxSize);
            RC_ASSERT(params.maxDiscardRatio == maxDiscardRatio);
        });
}
