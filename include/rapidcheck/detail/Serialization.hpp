#include "Serialization.h"

namespace rc {
namespace detail {

template <typename T, typename Iterator>
Iterator serializeCompact(T value, Iterator output) {
  static_assert(std::is_integral<T>::value, "T must be integral");

  using UInt = typename std::make_unsigned<T>::type;
  auto uvalue = static_cast<UInt>(value);
  do {
    auto x = uvalue & 0x7F;
    uvalue = uvalue >> 7;
    *output = static_cast<std::uint8_t>((uvalue == 0) ? x : (x | 0x80));
    output++;
  } while (uvalue != 0);

  return output;
}

template <typename T, typename Iterator>
Iterator deserializeCompact(Iterator begin, Iterator end, T &output) {
  static_assert(std::is_integral<T>::value, "T must be integral");
  using UInt = typename std::make_unsigned<T>::type;

  UInt uvalue = 0;
  int nbits = 0;
  for (auto it = begin; it != end; it++) {
    auto x = static_cast<std::uint8_t>(*it);
    uvalue |= static_cast<UInt>(x & 0x7F) << nbits;
    nbits += 7;
    if ((x & 0x80) == 0) {
      output = static_cast<T>(uvalue);
      return ++it;
    }
  }

  return begin;
}

template <typename InputIterator, typename OutputIterator>
OutputIterator
serializeCompact(InputIterator begin, InputIterator end, OutputIterator output) {
  const std::uint64_t numElements = std::distance(begin, end);
  auto oit = serializeCompact(numElements, output);
  for (auto it = begin; it != end; it++) {
    oit = serializeCompact(*it, oit);
  }

  return oit;
}

template <typename T, typename InputIterator, typename OutputIterator>
std::pair<InputIterator, OutputIterator> deserializeCompact(
    InputIterator begin, InputIterator end, OutputIterator output) {
  auto lenResult = deserializeCompact<std::uint64_t>(begin, end);
  if (lenResult.second == begin) {
    return std::make_pair(begin, output);
  }

  const auto numElements = lenResult.first;
  auto iit = lenResult.second;
  auto oit = output;
  for (std::uint64_t i = 0; i < numElements; i++) {
    const auto p = deserializeCompact<T>(iit, end);
    if (p.second == iit) {
      return std::make_pair(begin, oit);
    }
    *oit = p.first;
    oit++;
    iit = p.second;
  }

  return std::make_pair(iit, oit);
}

} // namespace detail
} // namespace rc
