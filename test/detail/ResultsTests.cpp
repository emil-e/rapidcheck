#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/Generators.h"
#include "util/TemplateProps.h"

using namespace rc;
using namespace rc::detail;

TEST_CASE("TestCase") {
    SECTION("operator==/operator!=") {
        propConformsToEquals<TestCase>();
        PROP_REPLACE_MEMBER_INEQUAL(TestCase, size);
        PROP_REPLACE_MEMBER_INEQUAL(TestCase, seed);
    }

    SECTION("operator<<") {
        propConformsToOutputOperator<TestCase>();
    }
}

TEST_CASE("CaseResult") {
    SECTION("operator==/operator!=") {
        propConformsToEquals<CaseResult>();
        PROP_REPLACE_MEMBER_INEQUAL(CaseResult, type);
        PROP_REPLACE_MEMBER_INEQUAL(CaseResult, description);
    }

    SECTION("operator<<") {
        propConformsToOutputOperator<CaseResult>();
    }
}

TEST_CASE("SuccessResult") {
    SECTION("operator==/operator!=") {
        propConformsToEquals<SuccessResult>();
        PROP_REPLACE_MEMBER_INEQUAL(SuccessResult, numSuccess);
    }

    SECTION("operator<<") {
        propConformsToOutputOperator<SuccessResult>();
    }
}

TEST_CASE("FailureResult") {
    SECTION("operator==/operator!=") {
        propConformsToEquals<FailureResult>();
        PROP_REPLACE_MEMBER_INEQUAL(FailureResult, numSuccess);
        PROP_REPLACE_MEMBER_INEQUAL(FailureResult, failingCase);
        PROP_REPLACE_MEMBER_INEQUAL(FailureResult, description);
        PROP_REPLACE_MEMBER_INEQUAL(FailureResult, numShrinks);
        PROP_REPLACE_MEMBER_INEQUAL(FailureResult, counterExample);
    }

    SECTION("operator<<") {
        propConformsToOutputOperator<FailureResult>();
    }
}

TEST_CASE("GaveUpResult") {
    SECTION("operator==/operator!=") {
        propConformsToEquals<GaveUpResult>();
        PROP_REPLACE_MEMBER_INEQUAL(GaveUpResult, numSuccess);
        PROP_REPLACE_MEMBER_INEQUAL(GaveUpResult, description);
    }

    SECTION("operator<<") {
        propConformsToOutputOperator<GaveUpResult>();
    }
}
