#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/detail/Serialization.h"

#include "util/Meta.h"
#include "util/TypeListMacros.h"
#include "util/Serialization.h"

using namespace rc;
using namespace rc::detail;
using namespace rc::test;

TEST_CASE("serialization(integers)") {
  forEachType<SerializationProperties, RC_INTEGRAL_TYPES>();
}

TEST_CASE("serialization(std::string)") {
  SerializationProperties::exec<std::string>();

  SECTION("deserialize") {
    prop("throws SerializationException on too short input",
         [](const std::string &str) {
           std::vector<std::uint8_t> data;
           serialize(str, std::back_inserter(data));
           data.erase(end(data) - 1);
           std::string output;
           try {
             // TODO RC_ASSERT_THROWS_AS
             deserialize(begin(data), end(data), output);
           } catch (const SerializationException &) {
             RC_SUCCEED("Threw SerializationException");
           }
           RC_FAIL("Threw incorrect or not exception");
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

template <typename Iterator>
Iterator deserialize(Iterator begin, Iterator end, NonDeserializable &output) {
  return begin;
}

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

struct SerializeCompactProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>("returns an iterator past the written data",
                     [](T value) {
                       std::vector<std::uint8_t> data(11, 0xFF);
                       const auto it = serializeCompact(value, begin(data));
                       RC_ASSERT(*it == 0xFF);
                     });
  }
};

TEST_CASE("serializeCompact") {
  forEachType<SerializeCompactProperties, RC_INTEGRAL_TYPES>();
}

struct DeserializeCompactProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>("deserializes output of serializeCompact",
                     [](T value) {
                       std::vector<std::uint8_t> data;
                       serializeCompact(value, std::back_inserter(data));
                       T output;
                       deserializeCompact(begin(data), end(data), output);
                       RC_ASSERT(output == value);
                     });

    templatedProp<T>(
        "returns an iterator past the end of the deserialized data",
        [](T value) {
          std::vector<std::uint8_t> data(11, 0);
          const auto it = serializeCompact(value, begin(data));
          T output;
          const auto rit = deserializeCompact(begin(data), end(data), output);
          RC_ASSERT(rit == it);
        });

    templatedProp<T>("throws SerializationException if data has unexpected end",
                     [](T value) {
                       std::vector<std::uint8_t> data;
                       serializeCompact(value, std::back_inserter(data));
                       data.erase(end(data) - 1);
                       T output;
                       try {
                         deserializeCompact(begin(data), end(data), output);
                       } catch (const SerializationException &e) {
                         RC_SUCCEED("Threw SerializationException");
                       }
                       RC_FAIL("Threw wrong or no exception");
                     });
  }
};

TEST_CASE("deserializeCompact") {
  forEachType<DeserializeCompactProperties, RC_INTEGRAL_TYPES>();

  prop(
      "representation for a number is identical regardless of data type for "
      "unsigned",
      [](std::uint32_t value) {
        std::vector<std::uint8_t> data32;
        std::vector<std::uint8_t> data64;
        serializeCompact(value, std::back_inserter(data32));
        serializeCompact(static_cast<std::uint64_t>(value),
                         std::back_inserter(data64));
        RC_ASSERT(data32 == data64);
      });
}

struct SerializeCompactRangeProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>("returns an iterator past the written data",
                     [](const std::vector<T> &values) {
                       std::vector<std::uint8_t> data((values.size() + 1) * 11,
                                                      0xFF);
                       const auto it = serializeCompact(
                           begin(values), end(values), begin(data));
                       RC_ASSERT(*it == 0xFF);
                     });
  }
};

TEST_CASE("serializeCompact(range)") {
  forEachType<SerializeCompactRangeProperties, RC_INTEGRAL_TYPES>();
}

struct DeserializeCompactRangeProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>("deserializes output of serializeCompact",
                     [](const std::vector<T> &values) {
                       std::vector<std::uint8_t> data;
                       serializeCompact(begin(values),
                                        end(values),
                                        std::back_inserter(data));

                       std::vector<T> output;
                       deserializeCompact<T>(
                           begin(data), end(data), std::back_inserter(output));

                       RC_ASSERT(output == values);
                     });

    templatedProp<T>(
        "returns the appropriate iterators",
        [](const std::vector<T> &values) {
          std::vector<std::uint8_t> data((values.size() + 1) * 11, 0xFF);
          const auto dataEnd =
              serializeCompact(begin(values), end(values), begin(data));

          std::vector<T> output(values.size(), 0);
          const auto result =
              deserializeCompact<T>(begin(data), end(data), begin(output));
          RC_ASSERT(result.first == dataEnd);
          RC_ASSERT(result.second == end(output));
        });

    templatedProp<T>(
        "throws SerializationException if data has too few elements",
        [](const std::vector<T> &values) {
          std::vector<std::uint8_t> data;
          serializeCompact(
              begin(values), end(values), std::back_inserter(data));

          // Read the length prefix
          std::uint32_t length;
          const auto it = deserializeCompact(begin(data), end(data), length);
          // Erase it
          data.erase(begin(data), it);
          // Replace it
          const auto newLength = length + *gen::inRange<std::uint64_t>(1, 100);
          serializeCompact(newLength, std::inserter(data, begin(data)));

          // Now it should fail
          std::vector<T> output;
          try {
            deserializeCompact<T>(
                begin(data), end(data), std::back_inserter(output));
          } catch (const SerializationException &) {
            RC_SUCCEED("Threw SerializationException");
          }
          RC_FAIL("Threw wrong or no exception");
        });
  }
};

TEST_CASE("deserializeCompact(range)") {
  forEachType<DeserializeCompactRangeProperties, RC_INTEGRAL_TYPES>();

  SECTION("throws SerializationException if length prefix is invalid") {
    std::vector<std::uint8_t> data(1, 0x80);
    std::vector<std::uint64_t> output;
    REQUIRE_THROWS_AS(deserializeCompact<std::uint64_t>(
                          begin(data), end(data), std::back_inserter(output)),
                      SerializationException);
    ;
  }
}
