#pragma once

#include "rapidcheck/detail/ShowType.h"
#include "rapidcheck/gen/Common.h"

namespace rc {

template<typename T, typename Testable>
void templatedProp(const std::string &description, Testable testable)
{
    prop(description + " (" + detail::typeToString<T>() + ")",
         testable);
}


#define TEMPLATED_SECTION(tparam, description)  \
    SECTION(std::string(description) + " (" +   \
            detail::typeToString<tparam>() + ")")

//! So that we in templated tests can compare map pairs with their non-const-key
//! equivalents.
template<typename T1, typename T2>
bool operator==(const std::pair<const T1, T2> &lhs,
                const std::pair<T1, T2> &rhs)
{
    return (lhs.first == rhs.first) && (lhs.second == rhs.second);
}

//! Returns the size of the given container by counting them through iterators.
template<typename T>
typename T::size_type containerSize(const T &container)
{
    return std::distance(begin(container), end(container));
}

//! Returns the set difference between the two given containers as computed by
//! `std::set_difference`.
template<typename T, typename C1, typename C2>
std::vector<T> setDifference(const C1 &c1, const C2 &c2)
{
    std::vector<T> cs1(begin(c1), end(c1));
    std::sort(cs1.begin(), cs1.end());
    std::vector<T> cs2(begin(c2), end(c2));
    std::sort(cs2.begin(), cs2.end());
    std::vector<T> result;
    std::set_difference(cs1.begin(), cs1.end(),
                        cs2.begin(), cs2.end(),
                        std::back_inserter(result));
    return result;
}

//! Generates a value that is not the same as the given value and replaces it.
template<typename T>
void replaceWithDifferent(T &value)
{
    value = *gen::suchThat(
        gen::arbitrary<T>(),
        [&] (const T &x) { return x != value; });
}

template<typename T> struct DeepDecayType;

template<typename T>
using DeepDecay = typename DeepDecayType<T>::Type;

template<typename T>
struct DeepDecayType
{
    typedef Decay<T> Type;
};

template<typename T1, typename T2>
struct DeepDecayType<std::pair<T1, T2>>
{
    typedef std::pair<DeepDecay<T1>, DeepDecay<T2>> Type;
};

struct NonComparable
{
    NonComparable(const char *x)
        : value(x) {}

    std::string value;
};

} // namespace rc
