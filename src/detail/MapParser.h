#pragma once

#include <stdexcept>
#include <string>
#include <map>

namespace rc {
namespace detail {

class ParseException : public std::exception
{
public:
    //! C-tor.
    //!
    //! @param pos  The position at which the parse error occured.
    //! @param msg  A message describing the parse error.
    ParseException(std::string::size_type pos, std::string msg);

    //! Returns the position.
    std::string::size_type position() const;

    //! Returns the message.
    std::string message() const;

    const char *what() const noexcept override;

private:
    std::string::size_type m_pos;
    std::string m_msg;
    std::string m_what;
};

//! Parses a configuration string into an `std::map<std::string, std::string>`.
std::map<std::string, std::string> parseMap(const std::string &str);

//! Converts a map to a string.
//!
//! @param map          The map to convert.
//! @param doubleQuote  Whether to use double quotes or single quotes.
std::string mapToString(const std::map<std::string, std::string> &map,
                        bool doubleQuote = false);

} // namespace detail
} // namespace rc
