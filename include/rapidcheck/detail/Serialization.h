#pragma once

namespace rc {
namespace detail {

/// Serializes the given integer value in a compact form where only as many bits
/// as needed are used.
///
/// This is done by using only seven bits in each byte. When high bit is set, it
/// indicates that there are more bytes to read. This format is only useful if
/// the number of bits required is usually low. If the higher bits are usually
/// set, it will lead to inefficient storage.
///
/// @param value   The value to serialize.
/// @param output  An output iterator to write bytes to.
///
/// @return An iterator pointing to after the written data.
template <typename T, typename Iterator>
Iterator serializeCompact(T value, Iterator output);

/// Deserializes integers as serialized by `serializeCompact`.
///
/// @param begin   The start iterator of the data to deserialize.
/// @param end     The end iterator of the data to deserialize.
/// @param output  Reference to store the output into.
///
/// @return Iterator pointing to the rest of the data that was not consumed or
///         `begin` on deserialization error.
template <typename T, typename Iterator>
Iterator deserializeCompact(Iterator begin, Iterator end, T &output);

} // namespace detail
} // namespace rc

#include "Serialization.hpp"
