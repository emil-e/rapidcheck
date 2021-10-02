#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>

#include "detail/ReproduceListener.h"
#include "detail/StringSerialization.h"

#include "util/Generators.h"

using namespace rc;
using namespace rc::detail;

TEST_CASE("ReproduceListener") {
  prop("ignores results with empty id",
       [](TestMetadata metadata, const TestResult &result) {
         std::ostringstream os;
         ReproduceListener listener(os);
         metadata.id = std::string();
         listener.onTestFinished(metadata, result);
         RC_ASSERT(os.str().empty());
       });

  prop("output string on destruction contains entire reproduce map",
       [] {
         const auto reproduceMap = *gen::nonEmpty(
             gen::container<std::unordered_map<std::string, Reproduce>>(
                 gen::nonEmpty<std::string>(), gen::arbitrary<Reproduce>()));

         std::ostringstream os;

         {
           ReproduceListener listener(os);
           for (const auto &p : reproduceMap) {
             TestMetadata metadata;
             metadata.id = p.first;
             FailureResult failure;
             failure.reproduce = p.second;
             listener.onTestFinished(metadata, failure);
           }
         }

         const auto str = os.str();
         const auto pre = std::string("reproduce=");
         const auto i = str.find(pre);
         RC_ASSERT(i != std::string::npos);
         const auto start = i + pre.size();
         const auto end = str.find('"', start);
         RC_ASSERT(end != std::string::npos);
         const auto reproString = str.substr(start, end - start);

         RC_ASSERT(stringToReproduceMap(reproString) == reproduceMap);
       });
}
