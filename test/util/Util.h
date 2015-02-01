#pragma once

#include "rapidcheck/shrink/Shrink.h"
#include "rapidcheck/detail/ShowType.h"
#include "rapidcheck/gen/Generator.h"

namespace rc {

#define RC_SIGNED_INTEGRAL_TYPES                \
    char,                                       \
    short,                                      \
    int,                                        \
    long,                                       \
    long long

#define RC_UNSIGNED_INTEGRAL_TYPES              \
    unsigned char,                              \
    unsigned short,                             \
    unsigned int,                               \
    unsigned long,                              \
    unsigned long long

#define RC_INTEGRAL_TYPES RC_SIGNED_INTEGRAL_TYPES, RC_UNSIGNED_INTEGRAL_TYPES

#define RC_REAL_TYPES double, float

#define RC_SIGNED_TYPES RC_REAL_TYPES, RC_SIGNED_INTEGRAL_TYPES

#define RC_NUMERIC_TYPES RC_REAL_TYPES, RC_INTEGRAL_TYPES

#define RC_GENERIC_CONTAINERS(T)                            \
    std::vector<T>,                                         \
    std::deque<T>,                                          \
    std::forward_list<T>,                                   \
    std::list<T>,                                           \
    std::set<T>,                                            \
    std::map<T, T>,                                         \
    std::multiset<T>,                                       \
    std::multimap<T, T>,                                    \
    std::unordered_set<T>,                                  \
    std::unordered_map<T, T>,                               \
    std::unordered_multiset<T>,                             \
    std::unordered_multimap<T, T>

template<typename T, typename Testable>
void templatedProp(const std::string &description, Testable testable)
{
    prop(description + " (" + detail::typeToString<T>() + ")",
         testable);
}


#define TEMPLATED_SECTION(tparam, description)  \
    SECTION(std::string(description) + " (" +   \
            detail::typeToString<tparam>() + ")")

//! Returns true all the shrinks returned by the given iterators are equal.
template<typename T>
bool yieldsEqual(const shrink::IteratorUP<T> &it1,
                 const shrink::IteratorUP<T> &it2)
{
    while (it1->hasNext()) {
        if (!it2->hasNext() || (it1->next() != it2->next()))
            return false;
    }

    return !it2->hasNext();
}

//! Retrieves all elements from the iterator and returns them as a vector.
template<typename T>
std::vector<T> takeAll(const shrink::IteratorUP<T> &iterator)
{
    std::vector<T> items;
    while (iterator->hasNext())
        items.push_back(iterator->next());
    return items;
}

//! Returns the final element of the iterator.
template<typename T>
T finalShrink(const shrink::IteratorUP<T> &iterator)
{
    T value;
    while (iterator->hasNext())
        value = iterator->next();
    return value;
}

//! Returns the number of shrinks of the given iterator.
template<typename T>
size_t shrinkCount(const shrink::IteratorUP<T> &iterator)
{
    size_t n = 0;
    while (iterator->hasNext()) {
        iterator->next();
        n++;
    }
    return n;
}

//! Returns true if there is a shrink in the given shrink iterator that
//! satisfies the given predicate.
template<typename T, typename Predicate>
bool hasShrinkSuchThat(const shrink::IteratorUP<T> &iterator,
                       Predicate predicate)
{
    while (iterator->hasNext()) {
        if (predicate(iterator->next()))
            return true;
    }

    return false;
}


//! Returns true if there is a shrink that is equal to the given element.
template<typename T>
bool hasShrink(const shrink::IteratorUP<T> &iterator, const T &value)
{
    return hasShrinkSuchThat(
        iterator,
        [&] (const T &x) { return x == value; });
}


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

template<typename T> struct DeepDecay;

template<typename T>
using DeepDecayT = typename DeepDecay<T>::Type;

template<typename T>
struct DeepDecay
{
    typedef detail::DecayT<T> Type;
};

template<typename T1, typename T2>
struct DeepDecay<std::pair<T1, T2>>
{
    typedef std::pair<DeepDecayT<T1>, DeepDecayT<T2>> Type;
};

template<typename T>
using DeepDecayT = typename DeepDecay<T>::Type;

struct NonComparable
{
    NonComparable(const char *x)
        : value(x) {}

    std::string value;
};

struct Apple
{
    Apple(const char *x)
        : value(x) {}

    std::string value;
};

struct Orange
{
    Orange(const char *x)
        : value(x) {}

    std::string value;
};

// Apples and Oranges have comparison operators to compare each other
inline bool operator==(const Apple &a, const Orange &o)
{ return a.value == o.value; }
inline bool operator==(const Orange &o, const Apple &a) { return a == o; }
inline bool operator!=(const Apple &a, const Orange &o) { return !(a == o); }
inline bool operator!=(const Orange &o, const Apple &a) { return !(a == o); }

} // namespace rc
