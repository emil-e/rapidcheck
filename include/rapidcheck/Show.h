#pragma once

#include <map>
#include <vector>
#include <deque>
#include <forward_list>
#include <list>
#include <set>
#include <unordered_set>
#include <unordered_map>

namespace rc {

//! Outputs a representation of the given value to the given output stream. The
//! default is to use \c operator<< but you can override this representation by
//! providing overloads for different types.
//!
//! @param value  The value.
//! @param os     The output stream.
template<typename T>
void show(const T &value, std::ostream &os);

//! Displays `*p` along with address.
template<typename T>
void show(T *p, std::ostream &os);

//! Displays `p.get()`.
template<typename T>
void show(const std::unique_ptr<T> &p, std::ostream &os);

template<typename T1, typename T2>
void show(const std::pair<T1, T2> &pair, std::ostream &os);

template<typename ...Types>
void show(const std::tuple<Types...> &tuple, std::ostream &os);

//! Overloaded show to display strings in a more readable way.
void show(const std::string &value, std::ostream &os);

//! Delegates to the \c std::string version
void show(const char *value, std::ostream &os);

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
void show(const std::vector<T, Allocator> &vec, std::ostream &os);

template<typename T, typename Allocator>
void show(const std::deque<T, Allocator> &vec, std::ostream &os);

template<typename T, typename Allocator>
void show(const std::forward_list<T, Allocator> &value, std::ostream &os);

template<typename T, typename Allocator>
void show(const std::list<T, Allocator> &value, std::ostream &os);

template<typename Key, typename Compare, typename Allocator>
void show(const std::set<Key, Compare, Allocator> &value, std::ostream &os);

template<typename Key,
         typename T,
         typename Compare,
         typename Allocator>
void show(const std::map<Key, T, Compare, Allocator> &value, std::ostream &os);

template<typename Key, typename Compare, typename Allocator>
void show(const std::multiset<Key, Compare, Allocator> &value, std::ostream &os);

template<typename Key,
         typename T,
         typename Compare,
         typename Allocator>
void show(const std::multimap<Key, T, Compare, Allocator> &value,
          std::ostream &os);

template<typename Key,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
void show(const std::unordered_set<Key, Hash, KeyEqual, Allocator> &value,
          std::ostream &os);

template<typename Key,
         typename T,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
void show(const std::unordered_map<Key, T, Hash, KeyEqual, Allocator> &value,
          std::ostream &os);


template<typename Key,
         typename T,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
void show(const std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator> &value,
          std::ostream &os);

template<typename Key,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
void show(const std::unordered_multiset<Key, Hash, KeyEqual, Allocator> &value,
          std::ostream &os);

} // namespace rc

#include "detail/Show.hpp"
