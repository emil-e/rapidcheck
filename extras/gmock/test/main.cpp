#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>
#include <gmock/gmock.h>
#include <rapidcheck/catch.h>
#include <rapidcheck/gmock.h>

using namespace testing;
using namespace rc;
using namespace rc::detail;
using namespace rc::gmock;

namespace rc {

template <>
struct Arbitrary<::testing::TestPartResult> {
  static Gen<::testing::TestPartResult> arbitrary() {
    return gen::apply(
        [](::testing::TestPartResult::Type type,
           const std::string &filename,
           int line,
           const std::string &message) {
          return ::testing::TestPartResult(
              type, filename.c_str(), line, message.c_str());
        },
        gen::element(::testing::TestPartResult::kSuccess,
                     ::testing::TestPartResult::kNonFatalFailure,
                     ::testing::TestPartResult::kFatalFailure),
        gen::arbitrary<std::string>(),
        gen::nonNegative<int>(),
        gen::arbitrary<std::string>());
  }
};

bool isEqual(const char *s1, const char *s2) {
  if ((s1 == nullptr) && (s2 == nullptr)) {
    return true;
  }
  if ((s1 == nullptr) || (s2 == nullptr)) {
    return false;
  }
  return std::strcmp(s1, s2) == 0;
}

} // namespace rc

namespace testing {

// GTests operator<< implementation is broken so override it
void showValue(const ::testing::TestPartResult &result, std::ostream &os) {
  os << "TestPartResult";
}

} // namespace testing

namespace {

class MockPropertyContext : public PropertyContext {
public:
  bool reportResult(const CaseResult &result) override {
    lastResult = result;
    return true;
  }
  std::ostream &logStream() override { return std::cerr; }
  void addTag(std::string str) override {}

  CaseResult lastResult;
};

struct DummyMock {
  MOCK_METHOD1(foobar, bool(const std::string &));
};

struct MockListener : public ::testing::EmptyTestEventListener {
  MockListener()
      : lastResult(::testing::TestPartResult::kSuccess, "", 0, "") {}

  void OnTestPartResult(const ::testing::TestPartResult &result) override {
    wasCalled = true;
    lastResult = result;
  }

  bool wasCalled = false;
  ::testing::TestPartResult lastResult;
};

bool descriptionContains(const CaseResult &result, const std::string &str) {
  return result.description.find(str) != std::string::npos;
}

} // namespace

CATCH_TEST_CASE("RapidCheckListener") {
  prop("forwards OnTestPartResult when not in property",
       [](const ::testing::TestPartResult &result) {
         const auto mock = new MockListener();
         RapidCheckListener listener{
             std::unique_ptr<::testing::TestEventListener>(mock)};

         ImplicitScope newScope;
         listener.OnTestPartResult(result);
         RC_ASSERT(mock->wasCalled);
         RC_ASSERT(mock->lastResult.type() == result.type());
         RC_ASSERT(isEqual(mock->lastResult.file_name(), result.file_name()));
         RC_ASSERT(mock->lastResult.line_number() == result.line_number());
         RC_ASSERT(isEqual(mock->lastResult.message(), result.message()));
       });

  prop("does not forward non-fatal OnTestPartResult when in property",
       [](const ::testing::TestPartResult &result) {
         RC_PRE(result.type() != ::testing::TestPartResult::kFatalFailure);
         const auto mock = new MockListener();
         RapidCheckListener listener{
             std::unique_ptr<::testing::TestEventListener>(mock)};

         MockPropertyContext mockContext;
         ImplicitParam<param::CurrentPropertyContext> letContext(&mockContext);
         listener.OnTestPartResult(result);
         RC_ASSERT(!mock->wasCalled);
       });

  prop("reported CaseResult contains relevant information",
       [](const ::testing::TestPartResult &result) {
         RC_PRE(result.type() != ::testing::TestPartResult::kFatalFailure);
         const auto mock = new MockListener();
         RapidCheckListener listener{
             std::unique_ptr<::testing::TestEventListener>(mock)};

         MockPropertyContext mockContext;
         ImplicitParam<param::CurrentPropertyContext> letContext(&mockContext);
         listener.OnTestPartResult(result);

         RC_ASSERT((mockContext.lastResult.type == CaseResult::Type::Failure) ==
                   result.failed());
         if (result.file_name()) {
           RC_ASSERT(
               descriptionContains(mockContext.lastResult, result.file_name()));
           RC_ASSERT(descriptionContains(mockContext.lastResult,
                                         std::to_string(result.line_number())));
         }
         RC_ASSERT(
             descriptionContains(mockContext.lastResult, result.message()));
       });

  CATCH_SECTION("should fail on GMock expectation failure") {
    const auto property = toProperty([] {
      DummyMock mock;
      EXPECT_CALL(mock, foobar("foo")).WillOnce(Return(true));
    });

    const auto desc = property(Random(), 0).value();
    CATCH_REQUIRE(desc.result.type == CaseResult::Type::Failure);
  }
}

int main(int argc, char **const argv) {
  rc::gmock::RapidCheckListener::install();
  return Catch::Session().run(argc, argv);
}
