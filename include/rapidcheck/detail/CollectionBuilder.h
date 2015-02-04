#pragma once

#include <forward_list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

namespace rc {
namespace detail {

template<typename Container>
class BaseBuilder
{
public:
    Container &result();

protected:
    Container m_container;
};

template<typename Container>
class EmplaceBackBuilder : public BaseBuilder<Container>
{
public:
    template<typename T>
    bool add(T &&value);
};

template<typename Container>
class InsertEndBuilder : public BaseBuilder<Container>
{
public:
    template<typename T>
    bool add(T &&value);
};

template<typename Container>
class InsertAfterBuilder : public BaseBuilder<Container>
{
public:
    InsertAfterBuilder();

    template<typename T>
    bool add(T &&value);

private:
    typename Container::iterator m_iterator;
};

template<typename Container>
class ArrayBuilder : public BaseBuilder<Container>
{
public:
    ArrayBuilder();

    template<typename T>
    bool add(T &&value);

private:
    typename Container::iterator m_iterator;
};

template<typename Container>
class InsertKeyMaybeBuilder : public BaseBuilder<Container>
{
public:
    template<typename T>
    bool add(T &&key);
};

template<typename Container>
class InsertPairMaybeBuilder : public BaseBuilder<Container>
{
public:
    template<typename T>
    bool add(T &&key);
};


namespace test {

// Fallback, std::false_type acts our null value
std::false_type builderTypeTest(...);

// Types supporting insert_after(), mostly std::forward_list I suppose
template<typename T, typename = decltype(
    std::declval<T>().insert_after(
        std::declval<typename T::iterator>(),
        std::declval<typename T::value_type>()))>
InsertAfterBuilder<T> builderTypeTest(const T &);

// Types supporting insert(value) returning a pair of an iterator and a boolean
// indicating success
template<typename T, typename = decltype(
    !!std::declval<T>().insert(std::declval<typename T::value_type>()).second)>
InsertPairMaybeBuilder<T> builderTypeTest(const T &);

// Types supporting insert(value) with a boolean return indicating success
template<typename T, typename = decltype(
    !!std::declval<T>().insert(std::declval<typename T::value_type>()))>
InsertKeyMaybeBuilder<T> builderTypeTest(const T &);

// Types supporting push_back(value)
template<typename T, typename = decltype(
    std::declval<T>().emplace_back(std::declval<typename T::value_type>()))>
EmplaceBackBuilder<T> builderTypeTest(const T &);

// Gives us a suitable builder for T, or std::false_type if unknown. This base
// template uses the SFINAE junk above.
template<typename T>
struct SuitableBuilder
{
    typedef decltype(builderTypeTest(std::declval<T>())) Type;
};

// Specialization for std::array, dunno any more generic way to give it a
// suitable type
template<typename T, std::size_t N>
struct SuitableBuilder<std::array<T, N>>
{
    typedef ArrayBuilder<std::array<T, N>> Type;
};

} // namespace test

// This is what you want to actually use. Uses SuitableBuilder to check for a
// suitable builder but if that "returns" std::false_type, use
// InsertEndBuilder<T> as default.
template<typename T>
using CollectionBuilder = typename std::conditional<
    std::is_same<std::false_type,
                 typename test::SuitableBuilder<T>::Type>::value,
    InsertEndBuilder<T>,
    typename test::SuitableBuilder<T>::Type>::type;

} // namespace detail
} // namespace rc

#include "CollectionBuilder.hpp"
