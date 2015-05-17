#include "User.h"

#include <iostream>

bool operator==(const User &lhs, const User &rhs) {
  return (lhs.username == rhs.username) && (lhs.firstName == rhs.firstName) &&
      (lhs.lastName == rhs.lastName) && (lhs.age == rhs.age) &&
      (lhs.gender == rhs.gender) && (lhs.hometown == rhs.hometown);
}

bool operator!=(const User &lhs, const User &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const User &lhs, const User &rhs);

std::ostream &operator<<(std::ostream &os, const Gender &gender) {
  os << ((gender == Gender::Male) ? "male" : "female");
  return os;
}

std::ostream &operator<<(std::ostream &os, const User &user) {
  os << "{username='" << user.username << "', firstName='" << user.firstName
     << "', lastName='" << user.lastName << "', age=" << user.age
     << ", gender=" << user.gender << ", hometown='" << user.hometown << "'}";
  return os;
}

void serialize(const User &user, std::vector<std::string> &out) {
  out.push_back(user.username);
  out.push_back(user.firstName);
  out.push_back(user.lastName);
  out.push_back(std::to_string(user.age));
  out.push_back((user.gender == Gender::Male) ? "male" : "female");
  out.push_back(user.hometown);
}

std::vector<std::string>::const_iterator
deserialize(std::vector<std::string>::const_iterator start,
            std::vector<std::string>::const_iterator end,
            User &user) {
  if ((end - start) < 6) {
    return start;
  }

  user.username = start[0];
  user.firstName = start[1];
  user.lastName = start[2];
  try {
    user.age = std::stoi(start[3]);
  } catch (const std::logic_error &) {
    return start;
  }
  user.gender = (start[4] == "male") ? Gender::Male : Gender::Female;
  user.hometown = start[5];

  return start + 6;
}
