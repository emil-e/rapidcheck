#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>

using namespace rc;
using namespace rc::detail;

struct LogPropertyContext : public PropertyContext {
  bool reportResult(const CaseResult &result) override { return false; }
  std::ostream &logStream() override { return stream; }
  void addTag(std::string str) override {}

  std::ostringstream stream;
};

TEST_CASE("RC_LOG") {
  SECTION("RC_LOG()") {
    prop("logs to the current log stream",
         [](const std::string &str) {
           LogPropertyContext context;
           ImplicitParam<param::CurrentPropertyContext> letContext(&context);
           RC_LOG() << str;
           RC_ASSERT(context.stream.str() == str);
         });
  }

  SECTION("RC_LOG(std::string)") {
    prop("logs to the current log stream with newline",
         [](const std::string &str) {
           LogPropertyContext context;
           ImplicitParam<param::CurrentPropertyContext> letContext(&context);
           RC_LOG(str);
           RC_ASSERT(context.stream.str() == (str + "\n"));
         });
  }
}
