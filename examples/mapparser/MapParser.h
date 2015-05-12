#pragma once

#include <stdexcept>
#include <string>
#include <map>

class ParseException : public std::exception {
public:
  ParseException(std::string::size_type pos, const std::string &msg);
  std::string::size_type position() const;
  std::string message() const;
  const char *what() const noexcept override;

private:
  std::string::size_type m_pos;
  std::string m_msg;
  std::string m_what;
};

std::map<std::string, std::string> parseMap(const std::string &str);

std::string mapToString(const std::map<std::string, std::string> &map,
                        bool doubleQuote = false);
