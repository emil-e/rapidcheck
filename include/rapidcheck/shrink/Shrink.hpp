#pragma once

#include "rapidcheck/seq/Transform.h"
#include "rapidcheck/seq/Create.h"

namespace rc {
namespace shrink {

template<typename Container>
Seq<Container> removeChunks(Container elements)
{
    return seq::map(
        seq::subranges(0, elements.size()),
        [=](const std::pair<std::size_t, std::size_t> &range) {
            Container newElements;
            newElements.reserve(range.second - range.first);
            const auto start = begin(elements);
            const auto fin = end(elements);
            newElements.insert(end(newElements), start, start + range.first);
            newElements.insert(end(newElements), start + range.second, fin);
            return newElements;
        });
}

template<typename Container, typename Shrink>
Seq<Container> eachElement(Container elements, Shrink shrink)
{
    using T = typename Container::value_type;

    const auto size = std::distance(begin(elements), end(elements));
    return seq::mapcat(seq::range<std::size_t>(0, size), [=](std::size_t i) {
        return seq::map(shrink(*(begin(elements) + i)), [=](T &&shrinkValue) {
            auto newElements = elements;
            *(begin(newElements) + i) = std::move(shrinkValue);
            return newElements;
        });
    });
}

template<typename T>
Seq<T> towards(T value, T target)
{
    typedef typename std::make_unsigned<T>::type UInt;

    UInt maxDiff = (value < target) ? (target - value) : (value - target);
    auto diffs = seq::iterate(maxDiff, [](UInt diff) { return diff / 2; });
    auto shrinks = seq::map(std::move(diffs), [=](UInt x) -> T {
        return (value < target) ? (value + x) : (value - x);
    });
    return seq::takeWhile(std::move(shrinks), [=](T x) { return x != value; });
}

template<typename T>
Seq<T> integral(T value)
{
    if (value < 0) {
        // Drop the zero from towards and put that before the negation value
        // so we don't have duplicate zeroes
        return seq::concat(
            seq::just<T>(static_cast<T>(0),
                         static_cast<T>(-value)),
            seq::drop(1, shrink::towards<T>(value, 0)));
    }

    return shrink::towards<T>(value, 0);
}

template<typename T>
Seq<T> real(T value)
{
    std::vector<T> shrinks;

    if (value != 0)
        shrinks.push_back(0.0);

    if (value < 0)
        shrinks.push_back(-value);

    T truncated = std::trunc(value);
    if (std::abs(truncated) < std::abs(value))
        shrinks.push_back(truncated);

    return seq::fromContainer(shrinks);
}

Seq<bool> boolean(bool value)
{
    return value
        ? seq::just(false)
        : Seq<bool>();
}

} // namespace shrink
} // namespace rc
