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

template<typename T, typename Callable>
void forEach(Seq<T> seq, Callable callable)
{
    while (seq)
        callable(seq.next());
}

} // namespace seq
} // namespace rc
