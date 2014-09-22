#pragma once

#include <iostream>
#include <string>
#include <iomanip>

#include "Utility.h"

namespace rc {
namespace detail {

template<typename TupleT,
         std::size_t I = std::tuple_size<TupleT>::value>
struct TupleHelper;

template<std::size_t I>
struct TupleHelper<std::tuple<>, I>
{
    static void showTuple(const std::tuple<> &tuple, std::ostream &os) {}
};

template<typename TupleT>
struct TupleHelper<TupleT, 1>
{
    static void showTuple(const TupleT &tuple, std::ostream &os)
    { show(std::get<std::tuple_size<TupleT>::value - 1>(tuple), os); }
};

template<typename TupleT, std::size_t I>
struct TupleHelper
{
    static void showTuple(const TupleT &tuple, std::ostream &os)
    {
        show(std::get<std::tuple_size<TupleT>::value - I>(tuple), os);
        os << ", ";
        TupleHelper<TupleT, I - 1>::showTuple(tuple, os);
    }
};

} // namespace detail

template<typename T>
void show(const T &value, std::ostream &os)
{
    os << value;
}

void show(uint8_t value, std::ostream &os)
{
    os << int(value);
}

template<typename T>
void show(T *p, std::ostream &os)
{
    show(*p, os);
    auto flags = os.flags();
    os << " (" << std::hex << std::showbase << p << ")";
    os.flags(flags);
}

template<typename T>
void show(const std::unique_ptr<T> &p, std::ostream &os)
{
    show(p.get(), os);
}

template<typename T1, typename T2>
void show(const std::pair<T1, T2> &pair, std::ostream &os)
{
    os << "(";
    show(pair.first, os);
    os << ", ";
    show(pair.second, os);
    os << ")";
}

template<typename ...Types>
void show(const std::tuple<Types...> &tuple, std::ostream &os)
{
    os << "(";
    detail::TupleHelper<std::tuple<Types...>>::showTuple(tuple, os);
    os << ")";
}

template<typename Collection>
void showCollection(const std::string &prefix,
                    const std::string &suffix,
                    const Collection &collection,
                    std::ostream &os)
{
    os << prefix;
    auto cbegin = begin(collection);
    auto cend = end(collection);
    if (cbegin != cend) {
        show(*cbegin, os);
        for (auto it = ++cbegin; it != cend; it++) {
            os << ", ";
            show(*it, os);
        }
    }
    os << suffix;
}

template<typename T, typename Allocator>
void show(const std::vector<T, Allocator> &value, std::ostream &os)
{
    showCollection("[", "]", value, os);
}

template<typename T, typename Allocator>
void show(const std::deque<T, Allocator> &value, std::ostream &os)
{
    showCollection("[", "]", value, os);
}

template<typename T, typename Allocator>
void show(const std::forward_list<T, Allocator> &value, std::ostream &os)
{
    showCollection("[", "]", value, os);
}

template<typename T, typename Allocator>
void show(const std::list<T, Allocator> &value, std::ostream &os)
{
    showCollection("[", "]", value, os);
}

template<typename Key, typename Compare, typename Allocator>
void show(const std::set<Key, Compare, Allocator> &value, std::ostream &os)
{
    showCollection("{", "}", value, os);
}

template<typename Key,
         typename T,
         typename Compare,
         typename Allocator>
void show(const std::map<Key, T, Compare, Allocator> &value, std::ostream &os)
{
    showCollection("{", "}", value, os);
}

template<typename Key, typename Compare, typename Allocator>
void show(const std::multiset<Key, Compare, Allocator> &value, std::ostream &os)
{
    showCollection("{", "}", value, os);
}

template<typename Key,
         typename T,
         typename Compare,
         typename Allocator>
void show(const std::multimap<Key, T, Compare, Allocator> &value, std::ostream &os)
{
    showCollection("{", "}", value, os);
}


template<typename Key,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
void show(const std::unordered_set<Key, Hash, KeyEqual, Allocator> &value,
          std::ostream &os)
{
    showCollection("{", "}", value, os);
}

template<typename Key,
         typename T,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
void show(const std::unordered_map<Key, T, Hash, KeyEqual, Allocator> &value,
          std::ostream &os)
{
    showCollection("{", "}", value, os);
}

template<typename Key,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
void show(const std::unordered_multiset<Key, Hash, KeyEqual, Allocator> &value,
          std::ostream &os)
{
    showCollection("{", "}", value, os);
}

template<typename Key,
         typename T,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
void show(const std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator> &value,
          std::ostream &os)
{
    showCollection("{", "}", value, os);
}

template<typename CharT, typename Traits, typename Allocator>
void show(const std::basic_string<CharT, Traits, Allocator> &value,
          std::ostream &os)
{
    showCollection("\"", "\"", value, os);
}

template<typename T, std::size_t N>
void show(const std::array<T, N> &value, std::ostream &os)
{
    showCollection("[", "]", value, os);
}

} // namespace rc
