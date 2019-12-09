#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/detail/Serialization.h"

#include "util/Meta.h"
#include "util/TypeListMacros.h"
#include "util/Serialization.h"

using namespace rc;
using namespace rc::detail;
using namespace rc::test;

TEST_CASE("serialization(std::string)") {
  SerializationProperties::exec<std::string>();

  SECTION("deserialize") {
    prop("throws SerializationException on too short input",
         [](const std::string &str) {
           std::vector<std::uint8_t> data;
           serialize(str, std::back_inserter(data));
           data.erase(end(data) - 1);

           std::string output;
           RC_ASSERT_THROWS_AS(deserialize(begin(data), end(data), output),
                               SerializationException);
         });
  }
}

TEST_CASE("serialization(std::pair)") {
  forEachType<SerializationProperties,
              std::pair<std::string, std::string>,
              std::pair<std::uint64_t, std::string>>();
}

TEST_CASE("serialization(std::unordered_map)") {
  forEachType<SerializationProperties,
              std::unordered_map<std::string, std::string>,
              std::unordered_map<std::uint64_t, std::string>>();
}

TEST_CASE("serializeN") {
  prop("returns an iterator past the written data",
       [](const std::vector<std::uint32_t> &values) {
         std::vector<std::uint8_t> data(values.size() * 4, 0);
         const auto it = serializeN(begin(values), values.size(), begin(data));
         RC_ASSERT(it == end(data));
       });
}

namespace {

struct NonDeserializable {};

} // namespace

TEST_CASE("deserializeN") {
  prop("deserializes output of serialize",
       [](const std::vector<std::uint32_t> &values) {
         std::vector<std::uint8_t> data(values.size() * 4, 0);
         serializeN(begin(values), values.size(), begin(data));

         std::vector<std::uint32_t> output(values.size(), 0);
         deserializeN<std::uint32_t>(
             begin(data), end(data), values.size(), begin(output));
         RC_ASSERT(output == values);
       });

  prop("returns an iterator past the end of the consumed data",
       [](const std::vector<std::uint32_t> &values) {
         std::vector<std::uint8_t> data(values.size() * 4, 0);
         serializeN(begin(values), values.size(), begin(data));

         std::vector<std::uint32_t> output(values.size(), 0);
         const auto it = deserializeN<std::uint32_t>(
             begin(data), end(data), values.size(), begin(output));
         RC_ASSERT(it == end(data));
       });
}
