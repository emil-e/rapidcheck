#pragma once

#include <cxxabi.h>
#include <cstdlib>

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

std::string demangle(const char *name)
{
    std::string demangled(name);
    int status;
    size_t length;
    char *buf = abi::__cxa_demangle(name, NULL, &length, &status);
    if (status == 0)
        demangled = std::string(buf, length);
    free(buf);
    return demangled;
}

template<typename Collection>
void pushBackAll(Collection &collection)
{
    // Base case
}

template<typename Collection, typename Item, typename ...Items>
void pushBackAll(Collection &collection, Item &&item, Items &&...items)
{
    collection.push_back(std::forward<Item>(item));
    pushBackAll(collection, std::forward<Items>(items)...);
}

} // namespace detail
} // namespace rc
