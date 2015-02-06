#pragma once

#include "rapidcheck/arbitrary/Collection.h"

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
class Arbitrary<Fruit<Tag>> : public gen::Generator<Fruit<Tag>>
{
public:
    Fruit<Tag> generate() const override
    { return Fruit<Tag>((*gen::arbitrary<std::string>()).c_str()); }
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
