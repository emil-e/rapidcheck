#pragma once

namespace rc {
namespace detail {

template<typename Collection>
void CollectionBuilder<Collection>::append(
    typename Collection::value_type value)
{
    m_collection.insert(m_collection.end(), std::move(value));
}

template<typename Collection>
Collection &CollectionBuilder<Collection>::collection()
{ return m_collection; }

template<typename T, typename Allocator>
CollectionBuilder<std::forward_list<T, Allocator>>::CollectionBuilder()
    : m_iterator(m_collection.before_begin()) {}

template<typename T, typename Allocator>
void CollectionBuilder<std::forward_list<T, Allocator>>::append(T value)
{ m_iterator = m_collection.insert_after(m_iterator, std::move(value)); }

template<typename T, typename Allocator>
std::forward_list<T, Allocator> &
CollectionBuilder<std::forward_list<T, Allocator>>::collection()
{ return m_collection; }

template<typename Map>
template<typename PairT>
void MapBuilder<Map>::append(PairT &&pair)
{ m_map.emplace(std::move(pair.first), std::move(pair.second)); }

template<typename Map>
Map &MapBuilder<Map>::collection()
{ return m_map; }

} // namespace detail
} // namespace rc
