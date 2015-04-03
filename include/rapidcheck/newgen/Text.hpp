#pragma once

#include <cctype>

#include "rapidcheck/detail/BitStream.h"
#include "rapidcheck/newgen/Container.h"

namespace rc {
namespace newgen {

template<typename T>
Gen<T> character()
{
    return [](const Random &random, int size) {
        auto stream = ::rc::detail::bitStreamOf(random);
        bool small = stream.next<bool>();
        T value;
        while ((value = small ? stream.next<T>(7) : stream.next<T>()) == '\0');

        return shrinkable::shrinkRecur(value, [](T value) {
            auto shrinks = seq::cast<T>(
                seq::concat(
                    seq::fromContainer(std::string("abc")),
                    std::islower(value)
                    ? Seq<char>()
                    : seq::just(static_cast<char>(std::tolower(value))),
                    seq::fromContainer(std::string("ABC123 \n"))));

            return seq::takeWhile(std::move(shrinks),
                                  [=](T x) { return x != value; });
        });
    };
}

namespace detail {

template<typename T, typename ...Args>
struct DefaultArbitrary<std::basic_string<T, Args...>>
{
    static Gen<std::basic_string<T, Args...>> arbitrary()
    {
        return newgen::container<std::basic_string<T, Args...>>(
            newgen::character<T>());
    }
};

} // namespace detail
} // namespace newgen
} // namespace rc
