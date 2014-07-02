#pragma once

#include <type_traits>
#include <forward_list>
#include <list>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <deque>
#include <stack>

namespace rc {
namespace detail {

//! Replacement for `std::is_copy_constructible` with specializations for
//! container types. `std::vector<std::unique_ptr<int>>` is copy constructible
//! according `std::is_copy_constructible` for example.
template<typename T>
struct IsCopyConstructible : public std::is_copy_constructible<T> {};

template<typename T, std::size_t N>
struct IsCopyConstructible<std::array<T, N>>
    : public IsCopyConstructible<T> {};

template<typename T, typename Allocator>
struct IsCopyConstructible<std::vector<T, Allocator>>
    : public IsCopyConstructible<T> {};

template<typename T, typename Allocator>
struct IsCopyConstructible<std::deque<T, Allocator>>
    : public IsCopyConstructible<T> {};

template<typename T, typename Allocator>
struct IsCopyConstructible<std::forward_list<T, Allocator>>
    : public IsCopyConstructible<T> {};

template<typename T, typename Allocator>
struct IsCopyConstructible<std::list<T, Allocator>>
    : public IsCopyConstructible<T> {};

template<typename Key, typename Compare, typename Allocator>
struct IsCopyConstructible<std::set<Key, Compare, Allocator>>
    : public IsCopyConstructible<Key> {};

template<typename Key, typename T, typename Compare, typename Allocator>
struct IsCopyConstructible<std::map<Key, T, Compare, Allocator>>
    : public std::integral_constant<bool,
                                    IsCopyConstructible<Key>::value &&
                                    IsCopyConstructible<T>::value> {};

template<typename Key, typename Compare, typename Allocator>
struct IsCopyConstructible<std::multiset<Key, Compare, Allocator>>
    : public IsCopyConstructible<Key> {};

template<typename Key, typename T, typename Compare, typename Allocator>
struct IsCopyConstructible<std::multimap<Key, T, Compare, Allocator>>
    : public std::integral_constant<bool,
                                    IsCopyConstructible<Key>::value &&
                                    IsCopyConstructible<T>::value> {};

template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
struct IsCopyConstructible<std::unordered_set<Key, Hash, KeyEqual, Allocator>>
    : public IsCopyConstructible<Key> {};

template<typename Key,
         typename T,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
struct IsCopyConstructible<std::unordered_map<Key, T, Hash, KeyEqual, Allocator>>
    : public std::integral_constant<bool,
                                    IsCopyConstructible<Key>::value &&
                                    IsCopyConstructible<T>::value> {};


template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
struct IsCopyConstructible<
    std::unordered_multiset<Key, Hash, KeyEqual, Allocator>>
    : public IsCopyConstructible<Key> {};

template<typename Key,
         typename T,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
struct IsCopyConstructible<
    std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>>
    : public std::integral_constant<bool,
                                    IsCopyConstructible<Key>::value &&
                                    IsCopyConstructible<T>::value> {};

template<typename T, typename Container>
struct IsCopyConstructible<std::stack<T, Container>>
    : public std::integral_constant<bool,
                                    IsCopyConstructible<T>::value &&
                                    IsCopyConstructible<Container>::value> {};

template<typename T, typename Container>
struct IsCopyConstructible<std::queue<T, Container>>
    : public std::integral_constant<bool,
                                    IsCopyConstructible<T>::value &&
                                    IsCopyConstructible<Container>::value> {};

template<typename T, typename Container, typename Compare>
struct IsCopyConstructible<std::priority_queue<T, Container, Compare>>
    : public std::integral_constant<bool,
                                    IsCopyConstructible<T>::value &&
                                    IsCopyConstructible<Container>::value> {};

template<typename ...Types>
struct IsCopyConstructible<std::tuple<Types...>>;

template<>
struct IsCopyConstructible<std::tuple<>> : public std::true_type {};

template<typename Type, typename ...Types>
struct IsCopyConstructible<std::tuple<Type, Types...>>
    : public std::integral_constant<
        bool,
        IsCopyConstructible<Type>::value &&
        IsCopyConstructible<std::tuple<Types...>>::value> {};

template<typename T1, typename T2>
struct IsCopyConstructible<std::pair<T1, T2>>
    : public IsCopyConstructible<std::tuple<T1, T2>> {};

//! Convenience wrapper over std::decay
template<typename T>
using DecayT = typename std::decay<T>::type;

} // namespace detail
} // namespace rc
