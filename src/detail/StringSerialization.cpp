#include "StringSerialization.h"

#include "Base64.h"
#include "ParseException.h"

namespace rc {
namespace detail {

std::string reproduceMapToString(
    const std::unordered_map<std::string, Reproduce> &reproduceMap) {
  std::vector<std::uint8_t> data;
  serialize(reproduceMap, std::back_inserter(data));
  return base64Encode(data);
}

std::unordered_map<std::string, Reproduce>
stringToReproduceMap(const std::string &str) {
  const auto data = base64Decode(str);
  std::unordered_map<std::string, Reproduce> reproduceMap;
#if RC_EXCEPTIONS_ENABLED
  try {
#endif
    deserialize(begin(data), end(data), reproduceMap);
#if RC_EXCEPTIONS_ENABLED
  } catch (const SerializationException &) {
    throw ParseException(0, "Invalid format");
  }
#endif

  return reproduceMap;
}

} // namespace detail
} // namespace rc
