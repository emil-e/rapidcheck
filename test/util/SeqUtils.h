#pragma once

#include "rapidcheck/seq/Operations.h"

namespace rc {
namespace test {

//! Forwards Seq a random amount and copies it to see if it is equal to the
//! original. Must not be infinite, of course.
template<typename T>
void assertEqualCopies(Seq<T> seq)
{
    std::size_t len = seq::length(seq);
    if (len != 0) {
        std::size_t n = *newgen::inRange<std::size_t>(0, len * 2);
        while (n--)
            seq.next();
    }
    const auto copy = seq;
    RC_ASSERT(copy == seq);

}

} // namespace test
} // namespace rc
