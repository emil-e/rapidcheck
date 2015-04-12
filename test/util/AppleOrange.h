#pragma once

#include "rapidcheck/gen/Arbitrary.h"
#include "rapidcheck/gen/Text.h"

namespace rc {

struct AppleTag {};
struct OrangeTag {};

template<typename Tag>
struct Fruit
{
    Fruit(const char *x)
        : value(x) {}

    std::string value;
};

template<typename Tag>
struct Arbitrary<Fruit<Tag>>
{
    static Gen<Fruit<Tag>> arbitrary()
    {
        return gen::map(
            gen::arbitrary<std::string>(),
            [](const std::string &s) {
                return Fruit<Tag>(s.c_str());
            });
    }
};

template<typename TagL, typename TagR>
inline bool operator==(const Fruit<TagL> &lhs, const Fruit<TagR> &rhs)
{ return lhs.value == rhs.value; }

template<typename TagL, typename TagR>
inline bool operator!=(const Fruit<TagL> &lhs, const Fruit<TagR> &rhs)
{ return !(lhs == rhs); }

template<typename TagL, typename TagR>
inline bool operator<(const Fruit<TagL> &lhs, const Fruit<TagR> &rhs)
{ return lhs.value < rhs.value; }

template<typename TagL, typename TagR>
inline bool operator>(const Fruit<TagL> &lhs, const Fruit<TagR> &rhs)
{ return lhs.value > rhs.value; }

template<typename TagL, typename TagR>
inline bool operator<=(const Fruit<TagL> &lhs, const Fruit<TagR> &rhs)
{ return lhs.value <= rhs.value; }

template<typename TagL, typename TagR>
inline bool operator>=(const Fruit<TagL> &lhs, const Fruit<TagR> &rhs)
{ return lhs.value >= rhs.value; }

typedef Fruit<AppleTag> Apple;
typedef Fruit<OrangeTag> Orange;

} // namespace rc
