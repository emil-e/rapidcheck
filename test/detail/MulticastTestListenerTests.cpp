#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "detail/MulticastTestListener.h"

#include "util/MockTestListener.h"
#include "util/Generators.h"

using namespace rc;
using namespace rc::detail;
using namespace rc::test;

namespace {

template <typename T>
MulticastTestListener makeUnicast(T listener) {
  MulticastTestListener::Listeners listeners;
  listeners.emplace_back(new T(std::move(listener)));
  return MulticastTestListener(std::move(listeners));
}

} // namespace

TEST_CASE("MulticastTestListener") {
  prop("calls all listeners exactly once",
       [] {
         const auto n = *gen::inRange<int>(0, 10);
         MulticastTestListener::Listeners listeners;
         std::vector<MockTestListener *> listenerPointers;
         for (int i = 0; i < n; i++) {
           auto listener =
               std::unique_ptr<MockTestListener>(new MockTestListener());
           listenerPointers.push_back(listener.get());
           listeners.emplace_back(std::move(listener));
         }

         MulticastTestListener listener(std::move(listeners));
         listener.onTestCaseFinished(CaseDescription());
         listener.onShrinkTried(CaseDescription(), true);
         listener.onTestFinished(TestMetadata(), SuccessResult());

         for (const auto *l : listenerPointers) {
           RC_ASSERT(l->onTestCaseFinishedCount == 1);
           RC_ASSERT(l->onShrinkTriedCount == 1);
           RC_ASSERT(l->onTestFinishedCount == 1);
         }
       });

  SECTION("onTestCaseFinished") {
    prop("passes on correct arguments", [](const CaseDescription &description) {
      MockTestListener mock;
      mock.onTestCaseFinishedCallback = [=](const CaseDescription &desc) {
        RC_ASSERT(desc == description);
      };
      auto listener = makeUnicast(mock);
      listener.onTestCaseFinished(description);
    });
  }

  SECTION("onShrinkTried") {
    prop("passes on correct arguments",
         [](const CaseDescription &shrink, bool accepted) {
           MockTestListener mock;
           mock.onShrinkTriedCallback = [=](const CaseDescription &shr,
                                            bool acc) {
             RC_ASSERT(shr == shrink);
             RC_ASSERT(acc == accepted);
           };
           auto listener = makeUnicast(mock);
           listener.onShrinkTried(shrink, accepted);
         });
  }

  SECTION("onTestFinished") {
    prop("passes on correct arguments",
         [](const TestMetadata &metadata, const TestResult &result) {
           MockTestListener mock;
           mock.onTestFinishedCallback = [=](const TestMetadata &meta,
                                             const TestResult &res) {
             RC_ASSERT(meta == metadata);
             RC_ASSERT(res == result);
           };
           auto listener = makeUnicast(mock);
           listener.onTestFinished(metadata, result);
         });
  }
}
