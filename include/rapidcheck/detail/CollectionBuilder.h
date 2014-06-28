#pragma once

#include <forward_list>
#include <map>
#include <unordered_map>

namespace rc {
namespace detail {

//! Helper class for building collections.
template<typename Collection>
class CollectionBuilder
{
public:
    //! Inserts an element at the end of the collection.
    void append(typename Collection::value_type value);

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
    void append(T value);
    std::forward_list<T, Allocator> &collection();

private:
    ListT m_collection;
    typename ListT::iterator m_iterator;
};

//! Base class for map specializations.
template<typename Map>
class MapBuilder
{
public:
    template<typename PairT>
    void append(PairT &&pair);
    Map &collection();

private:
    Map m_map;
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

} // namespace detail
} // namespace rc

#include "CollectionBuilder.hpp"
