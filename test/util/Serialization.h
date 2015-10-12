#pragma once

namespace rc {
namespace test {

struct SerializationProperties {
  template <typename T>
  static void exec() {
    using namespace rc::detail;

    templatedProp<T>("returns an iterator past the written data",
                     [](T value) {
                       std::vector<std::uint8_t> data;
                       serialize(value, std::back_inserter(data));
                       const auto it = serialize(value, begin(data));
                       RC_ASSERT(it == end(data));
                     });

    templatedProp<T>("deserializes output of serialize",
                     [](T value) {
                       std::vector<std::uint8_t> data;
                       serialize(value, std::back_inserter(data));
                       T output;
                       deserialize(begin(data), end(data), output);
                       RC_ASSERT(output == value);
                     });

    templatedProp<T>(
        "returns an iterator past the end of the deserialized data",
        [](T value) {
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
                       try {
                         // TODO RC_ASSERT_THROWS
                         deserialize(begin(data), end(data), output);
                       } catch (const SerializationException &e) {
                         RC_SUCCEED("Threw SerializationException");
                       }
                       RC_FAIL("Threw wrong or no exception");
                     });
  }
};

} // namespace test
} // namespace rc
