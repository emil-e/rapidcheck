#pragma once

#include <iostream>
#include <string>
#include <iomanip>

namespace rc {

//! Outputs a representation of the given value to the given output stream. The
//! default is to use \c operator<< but you can override this representation by
//! providing overloads for different types.
//!
//! @param value  The value.
//! @param os     The output stream.
template<typename T>
void show(const T &value, std::ostream &os)
{
    os << value;
}

//! Overloaded show to display strings in a more readable way.
void show(const std::string &value, std::ostream &os)
{
    os << '"';
    for (char c : value) {
        if (!std::isspace(c) && std::isprint(c)) {
            switch (c) {
            case '\\':
                os << "\\";
                break;
            case '"':
                os << "\\\"";
                break;
            default:
                os << c;
            }
        } else {
            switch (c) {
            case 0x00:
                os << "\\0";
                break;
            case 0x07:
                os << "\\a";
                break;
            case 0x08:
                os << "\\b";
                break;
            case 0x0C:
                os << "\\f";
                break;
            case 0x0A:
                os << "\\n";
                break;
            case 0x0D:
                os << "\\r";
                break;
            case 0x09:
                os << "\\t";
                break;
            case 0x0B:
                os << "\\v";
                break;
            case ' ':
                os << ' ';
                break;
            default:
                auto flags = os.flags();
                os << "\\x";
                os << std::hex << std::setprecision(2) << std::uppercase
                   << static_cast<int>(c & 0xFF);
                os.flags(flags);
            }
        }
    }
    os << '"';
}

//! Delegates to the \c std::string version
void show(const char *value, std::ostream &os)
{
    show(std::string(value), os);
}


//! Helper function for showing collections of values.
//!
//! @param prefix  The prefix to the collection, for example "["
//! @param suffix  The suffix to the collection, for example "]"
//! @param begin   The start of the range.
//! @param end     The end of the range.
//! @param os      The stream to output to.
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

} // namespace rc
