#pragma once

#include <string>
#include <tuple>

namespace rc {
namespace detail {

//! Disables copying
#define RC_DISABLE_COPY(Class)                    \
    Class(const Class &) = delete;                \
    Class &operator=(const Class &) = delete;

//! Disables moving
#define RC_DISABLE_MOVE(Class)            \
    Class(Class &&) = delete;             \
    Class &operator=(Class &&) = delete;

#define RC_GLUE2(a, b) a##b

//! Paste together the given arguments.
#define RC_GLUE(a, b) RC_GLUE2(a, b)

//! Unique identifier helper.
#define RC_UNIQUE(prefix) RC_GLUE(prefix, __LINE__)

//! Demangles a mangled C++
std::string demangle(const char *name);

//! Base case for `pushBackAll`
template<typename Collection>
void pushBackAll(Collection &collection)
{
    // Base case
}

//! Appends the given items to the end of the given collection.
//!
//! @param collection   The collection to append to.
//! @param item         The first item.
//! @param items        The rest of the items.
template<typename Collection, typename Item, typename ...Items>
void pushBackAll(Collection &collection, Item &&item, Items &&...items)
{
    collection.push_back(std::forward<Item>(item));
    pushBackAll(collection, std::forward<Items>(items)...);
}

//! Base case for `join`.
inline std::string join(const std::string &sep, const std::string str)
{
    return str;
}

//! Joins the given strings using the given separator.
//!
//! @param sep      The separator.
//! @param str      The first string.
//! @param strings  The rest of the strings.
template<typename ...Strings>
std::string join(const std::string &sep,
                 const std::string &str,
                 Strings ...strings)
{
    return str + sep + join(sep, strings...);
}

template<typename TupleT, typename ...Types>
struct TupleTailHelper;

template<typename TupleT>
struct TupleTailHelper<TupleT>
{
    static std::tuple<> tail(const TupleT &tuple)
    { return std::tuple<>(); }
};

template<typename TupleT, typename Type, typename ...Types>
struct TupleTailHelper<TupleT, Type, Types...>
{
    static std::tuple<Type, Types...> tail(const TupleT &tuple)
    {
        constexpr std::size_t tailHead =
            std::tuple_size<TupleT>::value - (sizeof...(Types) + 1);
        return std::tuple_cat(
            std::make_tuple(std::get<tailHead>(tuple)),
            TupleTailHelper<TupleT, Types...>::tail(tuple));
    }
};

//! Returns a copy of the given tuple without the first element.
template<typename Type, typename ...Types>
std::tuple<Types...> tupleTail(const std::tuple<Type, Types...> &tuple)
{
    return TupleTailHelper<std::tuple<Type, Types...>, Types...>::tail(tuple);
}

//! Avalanching function.
//! Written in 2014 by Sebastiano Vigna (vigna@acm.org)
inline uint64_t avalanche(uint64_t x) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    return x ^= x >> 33;
}

//! Returns a bitmask of the given type with the lowest `nbits` bits set to 1
//! and the rest set to 0.
template<typename T>
constexpr T bitMask(int nbits)
{ return ~((~static_cast<T>(0) - 1) << static_cast<T>(nbits - 1)); }

// TODO separate into header and implementation file

} // namespace detail
} // namespace rc
