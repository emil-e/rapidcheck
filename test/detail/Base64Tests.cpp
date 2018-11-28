#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>

#include "detail/Base64.h"
#include "detail/ParseException.h"

using namespace rc;
using namespace rc::detail;

TEST_CASE("base64") {
  prop("base64Decode(base64Encode(data)) == data",
       [](const std::vector<std::uint8_t> &data) {
         RC_ASSERT(base64Decode(base64Encode(data)) == data);
       });

  prop("throws error on decode with invalid length",
       [] {
         const auto n = *gen::inRange<std::size_t>(0, 20);
         const auto size = (n * 4) + 1;
         const auto data =
             *gen::container<std::string>(size, gen::arbitrary<char>());

         RC_ASSERT_THROWS_AS(base64Decode(data), ParseException);
       });

  prop("throws error on decode with invalid characters",
       [] {
         const auto alphabet = std::string(kBase64Alphabet);
         // TODO gen::string with allowed chars for teh speed
         auto data = *gen::container<std::string>(gen::elementOf(alphabet));
         const auto invalid = *gen::suchThat<char>([=](char c) {
           return alphabet.find(c) == std::string::npos;
         });
         const auto pos = *gen::inRange<std::size_t>(0, data.size() + 1);
         data.insert(begin(data) + pos, invalid);

         RC_ASSERT_THROWS_AS(base64Decode(data), ParseException);
       });
}
