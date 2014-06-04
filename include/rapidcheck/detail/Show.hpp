#pragma once

#include <iostream>
#include <string>
#include <iomanip>

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

} // namespace rc
