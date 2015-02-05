#pragma once

namespace rc {

// std::vector
template<typename T, typename Allocator>
class Arbitrary<std::vector<T, Allocator>>;

// std::deque
template<typename T, typename Allocator>
class Arbitrary<std::deque<T, Allocator>>;

// std::forward_list
template<typename T, typename Allocator>
class Arbitrary<std::forward_list<T, Allocator>>;

// std::list
template<typename T, typename Allocator>
class Arbitrary<std::list<T, Allocator>>;

// std::set
template<typename Key,
         typename Compare,
         typename Allocator>
class Arbitrary<std::set<Key, Compare, Allocator>>;

// std::map
template<typename Key,
         typename T,
         typename Compare,
         typename Allocator>
class Arbitrary<std::map<Key, T, Compare, Allocator>>;

// std::multiset
template<typename Key,
         typename Compare,
         typename Allocator>
class Arbitrary<std::multiset<Key, Compare, Allocator>>;

// std::multimap
template<typename Key,
         typename T,
         typename Compare,
         typename Allocator>
class Arbitrary<std::multimap<Key, T, Compare, Allocator>>;

// std::unordered_set
template<typename Key,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
class Arbitrary<std::unordered_set<Key, Hash, KeyEqual, Allocator>>;

// std::unordered_map
template<typename Key,
         typename T,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
class Arbitrary<std::unordered_map<Key, T, Hash, KeyEqual, Allocator>>;

// std::unordered_multiset
template<typename Key,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
class Arbitrary<std::unordered_multiset<Key, Hash, KeyEqual, Allocator>>;

// std::unordered_multimap
template<typename Key,
         typename T,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
class Arbitrary<std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>>;

// std::basic_string
template<typename T,
         typename Traits,
         typename Allocator>
class Arbitrary<std::basic_string<T, Traits, Allocator>>;

// std::array
template<typename T, std::size_t N>
class Arbitrary<std::array<T, N>>;

} // namespace rc

#include "Collection.hpp"
