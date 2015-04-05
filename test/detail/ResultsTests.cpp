#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/Generators.h"
#include "util/TemplateProps.h"

using namespace rc;
using namespace rc::detail;

TEST_CASE("TestCase") {
    SECTION("operator==/operator!=") {
        newpropConformsToEquals<TestCase>();
        NEWPROP_REPLACE_MEMBER_INEQUAL(TestCase, size);
        NEWPROP_REPLACE_MEMBER_INEQUAL(TestCase, seed);
    }

    SECTION("operator<<") {
        newpropConformsToOutputOperator<TestCase>();
    }
}

TEST_CASE("CaseResult") {
    SECTION("operator==/operator!=") {
        newpropConformsToEquals<CaseResult>();
        NEWPROP_REPLACE_MEMBER_INEQUAL(CaseResult, type);
        NEWPROP_REPLACE_MEMBER_INEQUAL(CaseResult, description);
    }

    SECTION("operator<<") {
        newpropConformsToOutputOperator<CaseResult>();
    }
}

TEST_CASE("SuccessResult") {
    SECTION("operator==/operator!=") {
        newpropConformsToEquals<SuccessResult>();
        NEWPROP_REPLACE_MEMBER_INEQUAL(SuccessResult, numSuccess);
    }

    SECTION("operator<<") {
        newpropConformsToOutputOperator<SuccessResult>();
    }
}

TEST_CASE("FailureResult") {
    SECTION("operator==/operator!=") {
        newpropConformsToEquals<FailureResult>();
        NEWPROP_REPLACE_MEMBER_INEQUAL(FailureResult, numSuccess);
        NEWPROP_REPLACE_MEMBER_INEQUAL(FailureResult, failingCase);
        NEWPROP_REPLACE_MEMBER_INEQUAL(FailureResult, description);
        NEWPROP_REPLACE_MEMBER_INEQUAL(FailureResult, numShrinks);
        NEWPROP_REPLACE_MEMBER_INEQUAL(FailureResult, counterExample);
    }

    SECTION("operator<<") {
        newpropConformsToOutputOperator<FailureResult>();
    }
}

TEST_CASE("GaveUpResult") {
    SECTION("operator==/operator!=") {
        newpropConformsToEquals<GaveUpResult>();
        NEWPROP_REPLACE_MEMBER_INEQUAL(GaveUpResult, numSuccess);
        NEWPROP_REPLACE_MEMBER_INEQUAL(GaveUpResult, description);
    }

    SECTION("operator<<") {
        newpropConformsToOutputOperator<GaveUpResult>();
    }
}
