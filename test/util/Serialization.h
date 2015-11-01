#pragma once

#include "util/Meta.h"

namespace rc {
namespace test {

struct SerializationProperties {
  template <typename T>
  static void exec(const Gen<T> &gen = gen::arbitrary<T>()) {
    using namespace rc::detail;

    templatedProp<T>("returns an iterator past the written data",
                     [&] {
                       const auto value = *gen;
                       std::vector<std::uint8_t> data;
                       serialize(value, std::back_inserter(data));
                       const auto it = serialize(value, begin(data));
                       RC_ASSERT(it == end(data));
                     });

    templatedProp<T>("deserializes output of serialize",
                     [&] {
                       const auto value = *gen;
                       std::vector<std::uint8_t> data;
                       serialize(value, std::back_inserter(data));
                       T output;
                       deserialize(begin(data), end(data), output);
                       RC_ASSERT(output == value);
                     });

    templatedProp<T>(
        "returns an iterator past the end of the deserialized data",
        [&] {
          const auto value = *gen;
          std::vector<std::uint8_t> data;
          serialize(value, std::back_inserter(data));
          T output;
          const auto it = deserialize(begin(data), end(data), output);
          RC_ASSERT(it == end(data));
        });

    templatedProp<T>("throws SerializationException if data has unexpected end",
                     [] {
                       std::vector<std::uint8_t> data;
                       T output;
                       RC_ASSERT_THROWS_AS(
                           deserialize(begin(data), end(data), output),
                           SerializationException);
                     });
  }
};

} // namespace test
} // namespace rc
