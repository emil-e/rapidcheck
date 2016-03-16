#pragma once

#include "rapidcheck/Gen.h"

namespace rc {
namespace gen {

/// Generator of text characters. Common occuring characters have a higher
/// probability of being generated.
template <typename T>
Gen<T> character();

/// Generator of Unicode Codepoint values. It has a higher chance
/// of generating lower value codepoints.
template <typename T>
Gen<T> unicodeCodepoint();

/// Generator of a container of Unicode Codepoint values.
template <typename Container>
Gen<Container> unicodeCodepoints();

/// Generator of Unicode text characters, encoded in utf8. 
/// Will return them in a string of variable length.
template <typename String>
Gen<String> characterUtf8();

/// Generator of strings. Essentially equivalent to
/// `gen::container<String>(gen::character<typename String::value_type>())` but
/// a lot faster. If you need to use a custom character generator, use
/// `gen::container`.
template <typename String>
Gen<String> string();

/// Generator of strings, as `gen::string<String>()`
/// but will be filled with utf8 encoded Unicode
template <typename String>
Gen<String> stringUtf8();


} // namespace gen
} // namespace rc

#include "Text.hpp"
