#pragma once

#include <iostream>
#include <string>
#include <iomanip>

#include "Utility.h"

namespace rc {

template<typename T>
void show(const T &value, std::ostream &os)
{
    os << value;
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

template<typename Iterator>
void showCollection(const std::string &prefix,
                    const std::string &suffix,
                    Iterator begin,
                    Iterator end,
                    std::ostream &os)
{
    os << prefix;
    if (begin != end) {
        show(*begin, os);
        for (auto it = ++begin; it != end; it++) {
            os << ", ";
            show(*it, os);
        }
    }
    os << suffix;
}

template<typename T, typename Alloc>
void show(const std::vector<T, Alloc> &vec, std::ostream &os)
{
    showCollection("[", "]", vec.begin(), vec.end(), os);
}

template<typename Key,
         typename T,
         typename Compare,
         typename Allocator>
void show(const std::map<Key, T, Compare, Allocator> &m, std::ostream &os)
{
    showCollection("{", "}", m.begin(), m.end(), os);
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

namespace detail {

template<typename TupleT,
         std::size_t I = std::tuple_size<TupleT>::value>
struct TupleHelper;

template<size_t I>
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

template<typename ...Types>
void show(const std::tuple<Types...> &tuple, std::ostream &os)
{
    os << "(";
    detail::TupleHelper<std::tuple<Types...>>::showTuple(tuple, os);
    os << ")";
}

} // namespace rc
