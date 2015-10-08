#include <catch.hpp>
#include <rapidcheck/catch.h>

#include <sstream>

#include "util/Generators.h"
#include "util/TemplateProps.h"

using namespace rc;
using namespace rc::detail;

namespace {

template <typename T>
bool messageContains(const T &msg, const std::string &substr) {
  std::ostringstream ss;
  printResultMessage(msg, ss);
  return ss.str().find(substr) != std::string::npos;
}

} // namespace

TEST_CASE("CaseResult") {
  SECTION("operator==/operator!=") {
    propConformsToEquals<CaseResult>();
    PROP_REPLACE_MEMBER_INEQUAL(CaseResult, type);
    PROP_REPLACE_MEMBER_INEQUAL(CaseResult, description);
  }

  SECTION("operator<<") { propConformsToOutputOperator<CaseResult>(); }
}

TEST_CASE("SuccessResult") {
  SECTION("operator==/operator!=") {
    propConformsToEquals<SuccessResult>();
    PROP_REPLACE_MEMBER_INEQUAL(SuccessResult, numSuccess);
    PROP_REPLACE_MEMBER_INEQUAL(SuccessResult, distribution);
  }

  SECTION("operator<<") { propConformsToOutputOperator<SuccessResult>(); }

  SECTION("printResultMessage") {
    prop("message contains relevant parts of the result",
         [](const SuccessResult &result) {
           RC_ASSERT(messageContains(result, "OK"));
           RC_ASSERT(
               messageContains(result, std::to_string(result.numSuccess)));

           if (!result.distribution.empty()) {
             const auto someTag =
                 gen::mapcat(gen::elementOf(result.distribution),
                             [](const std::pair<const Tags, int> &entry) {
                               return gen::elementOf(entry.first);
                             });
             RC_ASSERT(messageContains(result, *someTag));
           }
         });
  }
}

TEST_CASE("FailureResult") {
  SECTION("operator==/operator!=") {
    propConformsToEquals<FailureResult>();
    PROP_REPLACE_MEMBER_INEQUAL(FailureResult, numSuccess);
    PROP_REPLACE_MEMBER_INEQUAL(FailureResult, description);
    PROP_REPLACE_MEMBER_INEQUAL(FailureResult, reproduce);
    PROP_REPLACE_MEMBER_INEQUAL(FailureResult, counterExample);
  }

  SECTION("operator<<") { propConformsToOutputOperator<FailureResult>(); }

  SECTION("printResultMessage") {
    prop("message contains relevant parts of result",
         [](const FailureResult &result) {
           RC_ASSERT(
               messageContains(result, std::to_string(result.numSuccess + 1)));
           RC_ASSERT(messageContains(result, result.description));
           RC_ASSERT(
               result.reproduce.shrinkPath.empty() ||
               messageContains(
                   result, std::to_string(result.reproduce.shrinkPath.size())));
           for (const auto &item : result.counterExample) {
             messageContains(result, item.first);
             messageContains(result, item.second);
           }
         });
  }
}

TEST_CASE("GaveUpResult") {
  SECTION("operator==/operator!=") {
    propConformsToEquals<GaveUpResult>();
    PROP_REPLACE_MEMBER_INEQUAL(GaveUpResult, numSuccess);
    PROP_REPLACE_MEMBER_INEQUAL(GaveUpResult, description);
  }

  SECTION("operator<<") { propConformsToOutputOperator<GaveUpResult>(); }

  SECTION("printResultMessage") {
    prop("message contains relevant parts of result",
         [](const GaveUpResult &result) {
           RC_ASSERT(
               messageContains(result, std::to_string(result.numSuccess)));
           RC_ASSERT(messageContains(result, result.description));
         });
  }
}
