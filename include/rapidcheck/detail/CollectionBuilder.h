#pragma once

#include <forward_list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

namespace rc {
namespace detail {

//! Helper class for building collections.
template<typename Collection>
class CollectionBuilder
{
public:
    //! Inserts an element at the end of the collection. Returns true if the
    //! item was successfully added or false if it was not.
    bool add(typename Collection::value_type value);

    //! Returns a reference to the collection.
    Collection &collection();

private:
    Collection m_collection;
};

//! Specialization for `std::forward_list`
template<typename T, typename Allocator>
class CollectionBuilder<std::forward_list<T, Allocator>>
{
public:
    typedef std::forward_list<T, Allocator> ListT;

    CollectionBuilder();
    bool add(T value);
    ListT &collection();

private:
    ListT m_collection;
    typename ListT::iterator m_iterator;
};

//! Specialization for `std::array`
template<typename T, std::size_t N>
class CollectionBuilder<std::array<T, N>>
{
public:
    typedef std::array<T, N> ArrayT;

    CollectionBuilder();
    bool add(T value);
    ArrayT &collection();

private:
    ArrayT m_array;
    typename ArrayT::iterator m_iterator;
};

//! Base class for map specializations.
template<typename Map>
class MapBuilder
{
public:
    template<typename PairT>
    bool add(PairT pair);
    Map &collection();

private:
    Map m_map;
};

//! Base class for set specializations.
template<typename Set>
class SetBuilder
{
public:
    bool add(typename Set::key_type key);
    Set &collection();

private:
    Set m_set;
};

template<typename Key, typename T, typename Compare, typename Allocator>
class CollectionBuilder<std::map<Key, T, Compare, Allocator>>
    : public MapBuilder<std::map<Key, T, Compare, Allocator>> {};

template<typename Key, typename T, typename Compare, typename Allocator>
class CollectionBuilder<std::multimap<Key, T, Compare, Allocator>>
    : public MapBuilder<std::multimap<Key, T, Compare, Allocator>> {};

template<typename Key,
         typename T,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
class CollectionBuilder<std::unordered_map<Key, T, Hash, KeyEqual, Allocator>>
    : public MapBuilder<std::unordered_map<Key, T, Hash, KeyEqual, Allocator>>
{
};

template<typename Key,
         typename T,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
class CollectionBuilder<std::unordered_multimap<Key, T, Hash,KeyEqual, Allocator>>
    : public MapBuilder<std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>>
{
};

template<typename Key, typename Compare, typename Allocator>
class CollectionBuilder<std::set<Key, Compare, Allocator>>
    : public SetBuilder<std::set<Key, Compare, Allocator>> {};

template<typename Key,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
class CollectionBuilder<std::unordered_set<Key, Hash, KeyEqual, Allocator>>
    : public SetBuilder<std::unordered_set<Key, Hash, KeyEqual, Allocator>>
{
};

} // namespace detail
} // namespace rc

#include "CollectionBuilder.hpp"
