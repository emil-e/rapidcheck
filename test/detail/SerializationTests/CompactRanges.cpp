#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/detail/Serialization.h"

#include "util/Meta.h"
#include "util/TypeListMacros.h"
#include "util/Serialization.h"

using namespace rc;
using namespace rc::detail;
using namespace rc::test;

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
          RC_ASSERT_THROWS_AS(deserializeCompact<T>(begin(data),
                                                    end(data),
                                                    std::back_inserter(output)),
                              SerializationException);
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
