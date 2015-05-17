#pragma once

#include "User.h"

rc::Gen<std::string> genFirstName() {
  return rc::gen::weightedOneOf<std::string>(
      {{4, rc::gen::arbitrary<std::string>()},
       {8, rc::gen::element<std::string>("John", "Chuck", "Kevin", "Oscar")},
       {8,
        rc::gen::element<std::string>("Jane", "Sarah", "Kate", "Elizabeth")}});
}

rc::Gen<std::string> genLastName() {
  return rc::gen::weightedOneOf<std::string>({
      {4, rc::gen::arbitrary<std::string>()},
      {16, rc::gen::element<std::string>("Johnson", "Smith", "Cook", "Jobs")},
  });
}

rc::Gen<std::string> genTown() {
  return rc::gen::weightedOneOf<std::string>({
      {4, rc::gen::arbitrary<std::string>()},
      {16,
       rc::gen::element<std::string>(
           "Stockholm", "New York", "San Francisco", "Gothenburg")},
  });
}

namespace rc {

template <>
struct Arbitrary<User> {
  static Gen<User> arbitrary() {
    return gen::build<User>(
        gen::set(&User::username, gen::nonEmpty<std::string>()),
        gen::set(&User::firstName, genFirstName()),
        gen::set(&User::lastName, genLastName()),
        gen::set(&User::age, gen::inRange(0, 100)),
        gen::set(&User::gender, gen::element(Gender::Male, Gender::Female)),
        gen::set(&User::hometown, genTown()));
  }
};

} // namespace rc
