#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/detail/Serialization.h"

#include "util/Meta.h"
#include "util/TypeListMacros.h"
#include "util/Serialization.h"

using namespace rc;
using namespace rc::detail;
using namespace rc::test;

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

                       RC_ASSERT_THROWS_AS(
                           deserializeCompact(begin(data), end(data), output),
                           SerializationException);
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
