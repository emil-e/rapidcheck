#pragma once

namespace rc {
namespace seq {

template<typename T>
Seq<T> drop(std::size_t n, Seq<T> seq)
{
    for (std::size_t i = 0; i < n; i++) {
        if (!seq.next())
            return Seq<T>();
    }

    return seq;
}

} // namespace seq
} // namespace rc
