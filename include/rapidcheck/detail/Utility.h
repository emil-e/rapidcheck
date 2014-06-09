#pragma once

#include <string>

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

} // namespace detail
} // namespace rc
