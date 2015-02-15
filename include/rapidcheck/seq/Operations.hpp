#pragma once

namespace rc {
namespace seq {

template<typename T>
std::size_t length(Seq<T> seq)
{
    std::size_t l = 0;
    while (seq.next())
        l++;

    return l;
}

template<typename T, typename Callable>
void forEach(Seq<T> seq, Callable callable)
{
    Maybe<T> value;
    while ((value = seq.next()))
        callable(*value);
}

} // namespace seq
} // namespace rc
