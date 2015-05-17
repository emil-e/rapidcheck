#pragma once

#include <string>
#include <vector>
#include <functional>

enum class Gender { Male, Female };

struct User {
  std::string username;
  std::string firstName;
  std::string lastName;
  int age;
  Gender gender;
  std::string hometown;
};

bool operator==(const User &lhs, const User &rhs);
bool operator!=(const User &lhs, const User &rhs);
std::ostream &operator<<(std::ostream &os, const Gender &gender);
std::ostream &operator<<(std::ostream &os, const User &user);

void serialize(const User &user, std::vector<std::string> &out);

std::vector<std::string>::const_iterator
deserialize(std::vector<std::string>::const_iterator start,
            std::vector<std::string>::const_iterator end,
            User &user);

namespace std {

template <>
struct hash<User> {
  typedef User argument_type;
  typedef std::size_t result_type;

  result_type operator()(const User &user) const {
    std::size_t h = 0;
    // Yeah I know, this is broken but it's just an example
    h ^= std::hash<std::string>()(user.username);
    h ^= std::hash<std::string>()(user.firstName);
    h ^= std::hash<std::string>()(user.lastName);
    h ^= std::hash<int>()(user.age);
    h ^= std::hash<bool>()((user.gender == Gender::Male) ? true : false);
    h ^= std::hash<std::string>()(user.hometown);

    return h;
  }
};

} // namespace std
