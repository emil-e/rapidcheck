#include <catch.hpp>
#include <rapidcheck-catch.h>

using namespace rc;
using namespace rc::detail;

struct TagCollector : public PropertyContext {
  bool reportResult(const CaseResult &) { return false; }
  void addTag(std::string str) override { tags.push_back(std::move(str)); }

  std::vector<std::string> tags;
};

TEST_CASE("RC_CLASSIFY") {
  SECTION("uses condition as tag if none specified") {
    TagCollector collector;
    ImplicitParam<param::CurrentPropertyContext> letContext(&collector);
    RC_CLASSIFY(10 > 1);
    REQUIRE(collector.tags == std::vector<std::string>{"10 > 1"});
  }

  SECTION("does not tag if condition is false") {
    TagCollector collector;
    ImplicitParam<param::CurrentPropertyContext> letContext(&collector);
    RC_CLASSIFY(false);
    REQUIRE(collector.tags.empty());
  }

  prop("uses tags if provided",
       [](const std::string &tag1, int tag2) {
         TagCollector collector;
         ImplicitParam<param::CurrentPropertyContext> letContext(&collector);
         RC_CLASSIFY(true, tag1, tag2);
         RC_ASSERT(collector.tags ==
                   (std::vector<std::string>{toString(tag1), toString(tag2)}));
       });
}

TEST_CASE("RC_TAG") {
  prop("adds given tags",
       [](const std::string &tag1, int tag2) {
         TagCollector collector;
         ImplicitParam<param::CurrentPropertyContext> letContext(&collector);
         RC_TAG(tag1, tag2);
         RC_ASSERT(collector.tags ==
                   (std::vector<std::string>{toString(tag1), toString(tag2)}));
       });
}
