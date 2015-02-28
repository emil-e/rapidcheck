#pragma once

#include <cstdint>

namespace rc {
namespace detail {

//! This is a helper class to optimize the number of random numbers requested
//! from a random source. The random source must have a method `next()` that
//! returns a random number.
template<typename Source>
class BitStream
{
public:
    explicit BitStream(Source &source);

    //! Returns the next random of the given type and number of bits.
    template<typename T>
    T next(int nbits);

    //! Returns the next random of the given type and size. Size maxes out at
    //! `kNominalSize`.
    template<typename T>
    T nextWithSize(int size);

private:
    Source &m_source;
    uint64_t m_bits;
    int m_numBits;
};

//! Returns a bitstream with the given source.
template<typename Source>
BitStream<Source> bitStreamOf(Source &source);

} // namespace detail
} // namespace rc

#include "BitStream.hpp"
