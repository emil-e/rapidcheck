#pragma once

#include "rapidcheck/seq/Transform.h"
#include "rapidcheck/seq/Create.h"
#include "rapidcheck/detail/CollectionBuilder.h"

namespace rc {
namespace shrink {

template<typename Container>
Seq<Container> newRemoveChunks(Container elements)
{
    using Range = std::pair<std::size_t, std::size_t>;

    std::size_t size = elements.size();
    auto ranges = seq::mapcat(
        seq::range<std::size_t>(size, 0),
        [=](std::size_t rangeSize) {
            return seq::map(
                seq::range<std::size_t>(0, size - rangeSize + 1),
                [=](std::size_t rangeStart) {
                    return std::make_pair(rangeStart,
                                          rangeStart + rangeSize);
                });
        });

    return seq::map(std::move(ranges), [=](const Range &range) {
        Container newElements;
        newElements.reserve(range.second - range.first);
        const auto start = begin(elements);
        const auto fin = end(elements);
        newElements.insert(end(newElements), start, start + range.first);
        newElements.insert(end(newElements), start + range.second, fin);
        return newElements;
    });
}

template<typename T>
Seq<T> removeChunks(T collection)
{
    std::vector<typename T::value_type> elements;
    for (auto &&item : collection)
        elements.push_back(std::move(item));

    std::size_t size = elements.size();
    auto ranges = seq::mapcat(
        seq::range<std::size_t>(size, 0),
        [=](std::size_t rangeSize) {
            return seq::map(
                seq::range<std::size_t>(0, size - rangeSize + 1),
                [=](std::size_t rangeStart) {
                    return std::make_pair(rangeStart,
                                          rangeStart + rangeSize);
                });
        });

    return seq::map(
        std::move(ranges),
        [=](const std::pair<std::size_t, std::size_t> &range) {
            detail::CollectionBuilder<T> builder;
            for (std::size_t i = 0; i < range.first; i++)
                builder.add(elements[i]);
            for (std::size_t i = range.second; i < size; i++)
                builder.add(elements[i]);
            return std::move(builder.result());
        });
}

template<typename Container, typename Shrink>
Seq<Container> newEachElement(Container elements, Shrink shrink)
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

template<typename T, typename ShrinkElement>
Seq<T> eachElement(T collection, ShrinkElement shrinkElement)
{
    typedef typename std::result_of<
        ShrinkElement(typename T::value_type)>::type::ValueType
        ElementT;

    // TODO lazier
    std::vector<ElementT> elements;
    for (auto &&item : collection)
        elements.push_back(std::move(item));

    return seq::join(seq::zipWith(
        [elements, shrinkElement](std::size_t index, ElementT &&value) {
            // TODO this capturing... safe to capture by reference?
            return seq::mapMaybe(
                shrinkElement(std::move(value)),
                [&elements, index](ElementT &&shrinkValue) -> Maybe<T> {
                    detail::CollectionBuilder<T> builder;
                    for (std::size_t i = 0; i < elements.size(); i++) {
                        bool success = (i == index) ? builder.add(shrinkValue)
                            : builder.add(elements[i]);
                        if (success)
                            return Nothing;
                    }
                    return std::move(builder.result());
                });
        },
        seq::index(), seq::fromContainer(elements)));
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
