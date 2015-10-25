#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "detail/StringSerialization.h"

#include "util/Generators.h"

using namespace rc;
using namespace rc::detail;

TEST_CASE("stringToReproduceMap") {
  prop("deserializes what reproduceToString serialized",
       [](const std::unordered_map<std::string, Reproduce> &reproMap) {
         RC_ASSERT(stringToReproduceMap(reproduceMapToString(reproMap)) ==
                   reproMap);
       });
}
