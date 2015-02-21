#pragma once

#include "rapidcheck/seq/Transform.h"
#include "rapidcheck/seq/Create.h"
#include "rapidcheck/detail/CollectionBuilder.h"

namespace rc {
namespace newshrink {

template<typename T>
Seq<T> removeChunks(T collection)
{
    std::vector<typename T::value_type> elements;
    for (auto &&item : collection)
        elements.push_back(std::move(item));

    std::size_t size = elements.size();
    auto ranges = seq::mapcat(
        [=](std::size_t rangeSize) {
            return seq::map([=](std::size_t rangeStart) {
                return std::make_pair(rangeStart,
                                      rangeStart + rangeSize);
            }, seq::range<std::size_t>(0, size - rangeSize + 1));
        },
        seq::range<std::size_t>(size, 0));

    return seq::map([=](const std::pair<std::size_t, std::size_t> &range) {
        detail::CollectionBuilder<T> builder;
        for (std::size_t i = 0; i < range.first; i++)
            builder.add(elements[i]);
        for (std::size_t i = range.second; i < size; i++)
            builder.add(elements[i]);
        return std::move(builder.result());
    },  std::move(ranges));
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

    return seq::mapcat(
        [=](std::size_t index, ElementT &&value) {
            // TODO this capturing... safe to capture by reference?
            auto maybeShrinks = seq::map([&elements, index](
                ElementT &&shrinkValue) -> Maybe<T>
            {
                detail::CollectionBuilder<T> builder;
                for (std::size_t i = 0; i < elements.size(); i++) {
                    bool success = (i == index)
                        ? builder.add(shrinkValue)
                        : builder.add(elements[i]);
                    if (success)
                        return Nothing;
                }
                return std::move(builder.result());
            }, shrinkElement(std::move(value)));

            return seq::map(
                [](Maybe<T> &&x) { return std::move(*x); },
                seq::filter([](const Maybe<T> &x) { return bool(x); },
                            std::move(maybeShrinks)));
        },
        seq::index(),
        seq::fromContainer(elements));
}

template<typename T>
Seq<T> towards(T value, T target)
{
    typedef typename std::make_unsigned<T>::type UInt;

    UInt maxDiff = (value < target) ? (target - value) : (value - target);
    auto diffs = seq::iterate(maxDiff, [](UInt diff) { return diff / 2; });
    auto shrinks = seq::map(
        [=](UInt x) -> T {
            return (value < target) ? (value + x) : (value - x);
        },
        std::move(diffs));
    return seq::takeWhile([=](T x) { return x != value; }, std::move(shrinks));
}

} // namespace newshrink
} // namespace rc
