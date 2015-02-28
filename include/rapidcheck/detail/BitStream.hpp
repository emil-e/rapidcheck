#pragma once

#include "rapidcheck/gen/Generator.h"
#include "rapidcheck/detail/Utility.h"

namespace rc {
namespace detail {

template<typename Source>
BitStream<Source>::BitStream(Source &source)
    : m_source(source)
    , m_bits(0)
    , m_numBits(0) {}

template<typename Source>
template<typename T>
T BitStream<Source>::next(int nbits)
{
    typedef decltype(m_source.next()) SourceType;
    typedef typename std::make_unsigned<SourceType>::type SourceUType;
    static constexpr auto sourceBits = std::numeric_limits<SourceUType>::digits;
    if (nbits == 0)
        return 0;

    T value = 0;
    int wantBits = nbits;
    while (wantBits > 0) {
        // Out of bits, refill
        if (m_numBits == 0) {
            m_bits = m_source.next();
            m_numBits += sourceBits;
        }

        int n = std::min(m_numBits, wantBits);
        T bits = m_bits & bitMask<SourceType>(n);
        value |= (bits << (nbits - wantBits));
        m_bits >>= static_cast<SourceType>(n);
        m_numBits -= n;
        wantBits -= n;
    }

    if (std::is_signed<T>::value) {
        T signBit = static_cast<T>(0x1) << static_cast<T>(nbits - 1);
        if ((value & signBit) != 0) {
            // For signed values, we use the last bit as the sign bit. Since this
            // was 1, mask away by setting all bits above this one to 1 to make it a
            // negative number in 2's complement
            value |= ~bitMask<T>(nbits);
        }
    }

    return value;
}

template<typename Source>
template<typename T>
T BitStream<Source>::nextWithSize(int size)
{
    typedef typename std::make_unsigned<T>::type UInt;
    constexpr auto maxBits = std::numeric_limits<UInt>::digits;
    return next<T>((size * maxBits) / gen::kNominalSize);
}

template<typename Source>
BitStream<Source> bitStreamOf(Source &source)
{ return BitStream<Source>(source); }

} // namespace detail
} // namespace rc
