#pragma once

#include "rapidcheck/gen/Collection.h"
#include "rapidcheck/gen/Text.h"

namespace rc {

// Base template class for collection types
template<typename Coll, typename ValueType>
class ArbitraryCollection : public gen::Collection<Coll, Arbitrary<ValueType>>
{
public:
    typedef gen::Collection<Coll, Arbitrary<ValueType>> CollectionGen;
    ArbitraryCollection() : CollectionGen(gen::arbitrary<ValueType>()) {}
};

// std::vector
template<typename T, typename Allocator>
class Arbitrary<std::vector<T, Allocator>>
    : public ArbitraryCollection<std::vector<T, Allocator>, T> {};

// std::deque
template<typename T, typename Allocator>
class Arbitrary<std::deque<T, Allocator>>
    : public ArbitraryCollection<std::deque<T, Allocator>, T> {};

// std::forward_list
template<typename T, typename Allocator>
class Arbitrary<std::forward_list<T, Allocator>>
    : public ArbitraryCollection<std::forward_list<T, Allocator>, T> {};

// std::list
template<typename T, typename Allocator>
class Arbitrary<std::list<T, Allocator>>
    : public ArbitraryCollection<std::list<T, Allocator>, T> {};

// std::set
template<typename Key, typename Compare, typename Allocator>
class Arbitrary<std::set<Key, Compare, Allocator>>
    : public ArbitraryCollection<std::set<Key, Compare, Allocator>, Key> {};

// std::map
template<typename Key, typename T, typename Compare, typename Allocator>
class Arbitrary<std::map<Key, T, Compare, Allocator>>
    : public ArbitraryCollection<std::map<Key, T, Compare, Allocator>,
                                 std::pair<Key, T>> {};

// std::multiset
template<typename Key, typename Compare, typename Allocator>
class Arbitrary<std::multiset<Key, Compare, Allocator>>
    : public ArbitraryCollection<std::multiset<Key, Compare, Allocator>, Key> {};

// std::multimap
template<typename Key, typename T, typename Compare, typename Allocator>
class Arbitrary<std::multimap<Key, T, Compare, Allocator>>
    : public ArbitraryCollection<std::multimap<Key, T, Compare, Allocator>,
                                 std::pair<Key, T>> {};

// std::unordered_set
template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
class Arbitrary<std::unordered_set<Key, Hash, KeyEqual, Allocator>>
    : public ArbitraryCollection<std::unordered_set<Key, Hash, KeyEqual, Allocator>,
                                 Key> {};

// std::unordered_map
template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
class Arbitrary<std::unordered_map<Key, T, Hash, KeyEqual, Allocator>>
    : public ArbitraryCollection<std::unordered_map<Key, T, Hash, KeyEqual, Allocator>,
                                 std::pair<Key, T>> {};

// std::unordered_multiset
template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
class Arbitrary<std::unordered_multiset<Key, Hash, KeyEqual, Allocator>>
    : public ArbitraryCollection<std::unordered_multiset<Key, Hash, KeyEqual, Allocator>,
                                 Key> {};

// std::unordered_multimap
template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
class Arbitrary<std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>>
    : public ArbitraryCollection<std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>,
                                 std::pair<Key, T>> {};

// std::basic_string
template<typename T, typename Traits, typename Allocator>
class Arbitrary<std::basic_string<T, Traits, Allocator>>
    : public gen::Collection<std::basic_string<T, Traits, Allocator>, gen::Character<T>>
{
public:
    typedef std::basic_string<T, Traits, Allocator> StringType;
    Arbitrary() : gen::Collection<StringType, gen::Character<T>>(
        gen::character<T>()) {}
};

// std::array
template<typename T, std::size_t N>
class Arbitrary<std::array<T, N>>
    : public ArbitraryCollection<std::array<T, N>, T> {};

} // namespace rc
