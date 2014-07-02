#pragma once

namespace rc {
namespace detail {

template<typename Collection>
bool CollectionBuilder<Collection>::add(
    typename Collection::value_type value)
{
    m_collection.insert(m_collection.end(), std::move(value));
    return true;
}

template<typename Collection>
Collection &CollectionBuilder<Collection>::collection()
{ return m_collection; }

template<typename T, typename Allocator>
CollectionBuilder<std::forward_list<T, Allocator>>::CollectionBuilder()
    : m_iterator(m_collection.before_begin()) {}

template<typename T, typename Allocator>
bool CollectionBuilder<std::forward_list<T, Allocator>>::add(T value)
{
    m_iterator = m_collection.insert_after(m_iterator, std::move(value));
    return true;
}

template<typename T, typename Allocator>
std::forward_list<T, Allocator> &
CollectionBuilder<std::forward_list<T, Allocator>>::collection()
{ return m_collection; }

template<typename Map>
template<typename PairT>
bool MapBuilder<Map>::add(PairT pair)
{
    if (m_map.count(pair.first) != 0)
        return false;

    m_map.emplace(std::move(pair.first), std::move(pair.second));
    return true;
}

template<typename Map>
Map &MapBuilder<Map>::collection()
{ return m_map; }

template<typename Set>
bool SetBuilder<Set>::add(typename Set::key_type key)
{
    if (m_set.count(key) != 0)
        return false;

    m_set.insert(std::move(key));
    return true;
}

template<typename Set>
Set &SetBuilder<Set>::collection()
{ return m_set; }

} // namespace detail
} // namespace rc
