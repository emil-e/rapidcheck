#pragma once

#include "Generator.h"

namespace rc {
namespace gen {

template<typename T>
class Character : public Generator<T>
{
public:
    T generate() const override
    {
        return *oneOf(map(ranged<uint8_t>(1, 128),
                          [](uint8_t x) { return static_cast<T>(x); }),
                      nonZero<T>());
    }

    Seq<T> shrink(T value) const override
    {
        // TODO improve?
        return seq::takeWhile(
            [=](T x) { return x != value; },
            seq::cast<T>(seq::fromContainer(std::string("abcABC123"))));
    }
};

template<typename T>
Character<T> character() { return Character<T>(); }

} // namespace gen
} // namespace rc
