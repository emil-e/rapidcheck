#pragma once

#include <string>

#include "rapidcheck/detail/BitStream.h"

namespace rc {
namespace detail {

/// By using a bitstream this function will return a single 
/// Unicode codepoint, with lower values having a higher chance
/// to appear than the higher ones. Most results will be
/// within the the basic multilingual plane, though
/// any valid Unicode codepoint may be generated.
template<typename T, typename RandomType>
T generateCodePoint(rc::detail::BitStream<RandomType>& stream);

/// Converts a codepoint into a string containing the utf8
/// encoding of passed codepoint.
template<typename T, typename Y>
T makeCharacterUtf8(Y codepoint);

} // namespace detail
} // namespace rc


#include "Unicode.hpp"
