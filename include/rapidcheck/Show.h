#pragma once

#include <map>
#include <vector>
#include <deque>
#include <forward_list>
#include <list>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <type_traits>
#include <array>

namespace rc {

template<typename T> struct Show;

//! Displays a byte as an integer.
static inline void showValue(uint8_t value, std::ostream &os);

//! Displays `*p` along with address.
template<typename T>
void showValue(T *p, std::ostream &os);

//! Displays `p.get()`.
template<typename T, typename Deleter>
void showValue(const std::unique_ptr<T, Deleter> &p, std::ostream &os);

//! Displays `p.get()`.
template<typename T>
void showValue(const std::shared_ptr<T> &p, std::ostream &os);

template<typename T1, typename T2>
void showValue(const std::pair<T1, T2> &pair, std::ostream &os);

template<typename ...Types>
void showValue(const std::tuple<Types...> &tuple, std::ostream &os);

//! Overloaded showValue to display strings in a more readable way.
void showValue(const std::string &value, std::ostream &os);

//! Delegates to the \c std::string version
void showValue(const char *value, std::ostream &os);

//! Helper function for showing collections of values.
//!
//! @param prefix      The prefix to the collection, for example "["
//! @param suffix      The suffix to the collection, for example "]"
//! @param collection  The collection type. Must support `begin()` and `end()`.
//! @param os          The stream to output to.
template<typename Collection>
void showCollection(const std::string &prefix,
                    const std::string &suffix,
                    const Collection &collection,
                    std::ostream &os);

template<typename T, typename Allocator>
void showValue(const std::vector<T, Allocator> &vec, std::ostream &os);

template<typename T, typename Allocator>
void showValue(const std::deque<T, Allocator> &vec, std::ostream &os);

template<typename T, typename Allocator>
void showValue(const std::forward_list<T, Allocator> &value, std::ostream &os);

template<typename T, typename Allocator>
void showValue(const std::list<T, Allocator> &value, std::ostream &os);

template<typename Key, typename Compare, typename Allocator>
void showValue(const std::set<Key, Compare, Allocator> &value,
               std::ostream &os);

template<typename Key,
         typename T,
         typename Compare,
         typename Allocator>
void showValue(const std::map<Key, T, Compare, Allocator> &value,
               std::ostream &os);

template<typename Key, typename Compare, typename Allocator>
void showValue(const std::multiset<Key, Compare, Allocator> &value,
               std::ostream &os);

template<typename Key,
         typename T,
         typename Compare,
         typename Allocator>
void showValue(const std::multimap<Key, T, Compare, Allocator> &value,
               std::ostream &os);

template<typename Key,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
void showValue(const std::unordered_set<Key, Hash, KeyEqual, Allocator> &value,
               std::ostream &os);

template<typename Key,
         typename T,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
void showValue(const std::unordered_map<Key, T, Hash, KeyEqual, Allocator> &value,
               std::ostream &os);

template<typename Key,
         typename T,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
void showValue(
    const std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator> &value,
    std::ostream &os);

template<typename Key,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
void showValue(
    const std::unordered_multiset<Key, Hash, KeyEqual, Allocator> &value,
    std::ostream &os);

template<typename T, std::size_t N>
void showValue(const std::array<T, N> &value, std::ostream &os);

//! Outputs a human readable representation of the given value to the given
//! output stream. To do this, it tries the following methods in order until one
//! works:
//!
//! 1. Use a suitable overload of `void showValue(T, std::ostream)´
//! 2. Use a suitable overload of `std::ostream &operator<<(...)`
//! 3. Output a placeholder value.
template<typename T>
void show(const T &value, std::ostream &os);

//! Uses show(...) to convert argument to a string.
template<typename T>
std::string toString(const T &value);

} // namespace rc

#include "Show.hpp"
