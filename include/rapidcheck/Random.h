#pragma once

#include <cstdint>
#include <array>
#include <limits>

namespace rc {

//! Implementation of a splittable random generator as described in:
//!   Claessen, K. och Palka, M. (2013) Splittable Pseudorandom Number
//!   Generators using Cryptographic Hashing.
class Random
{
public:
    //! Key type
    typedef std::array<uint64_t, 4> Key;

    Random(const Key &key);

    //! Creates a second generator from this one. Both `split` and `next` should
    //! not be called on the same state.
    Random split();

    //! Returns the next random number. Both `split` and `next` should not be
    //! called on the same state.
    uint64_t next();

private:
    typedef std::array<uint64_t, 4> Block;

    typedef uint64_t Bits;
    static constexpr auto kBits = std::numeric_limits<Bits>::digits;

    typedef uint64_t Counter;
    static constexpr auto kCounterMax = std::numeric_limits<Counter>::max();

    void append(bool x);
    void mash(Block &output);

    Block m_key;
    Block m_block;
    Bits m_bits;
    Counter m_counter;
    uint8_t m_bitsi;
};

} // namespace rc
