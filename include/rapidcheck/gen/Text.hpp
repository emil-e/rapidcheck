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

    shrink::IteratorUP<T> shrink(T value) const override
    {
        // TODO this can probably be better
        std::vector<T> chars;
        switch (value) {
        default:
            chars.insert(chars.begin(), static_cast<T>('3'));
        case '3':
            chars.insert(chars.begin(), static_cast<T>('2'));
        case '2':
            chars.insert(chars.begin(), static_cast<T>('1'));
        case '1':
            chars.insert(chars.begin(), static_cast<T>('C'));
        case 'C':
            chars.insert(chars.begin(), static_cast<T>('B'));
        case 'B':
            chars.insert(chars.begin(), static_cast<T>('A'));
        case 'A':
            chars.insert(chars.begin(), static_cast<T>('c'));
        case 'c':
            chars.insert(chars.begin(), static_cast<T>('b'));
        case 'b':
            chars.insert(chars.begin(), static_cast<T>('a'));
        case 'a':
            ;
        }

        return shrink::constant<T>(chars);
    }
};

template<typename T>
Character<T> character() { return Character<T>(); }

} // namespace gen
} // namespace rc
