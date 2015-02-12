#pragma once

namespace rc {
namespace seq {

template<typename T>
std::size_t length(Seq<T> seq)
{
    std::size_t l = 0;
    while (seq) {
        seq.next();
        l++;
    }

    return l;
}

} // namespace seq
} // namespace rc
