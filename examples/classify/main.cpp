#include <rapidcheck.h>

using namespace rc;

enum class Gender { Male, Female };

std::ostream &operator<<(std::ostream &os, Gender gender) {
  os << ((gender == Gender::Male) ? "Male" : "Female");
  return os;
}

struct User {
  std::string username;
  Gender gender;
};

namespace rc {

template <>
struct Arbitrary<User> {
  static Gen<User> arbitrary() {
    return gen::build<User>(
        gen::set(&User::username),
        gen::set(&User::gender, gen::element(Gender::Male, Gender::Female)));
  }
};

} // namespace rc

int main() {
  rc::check("RC_TAG", [](const User &user) { RC_TAG(user.gender); });

  rc::check("RC_CLASSIFY",
            [](const User &user) { RC_CLASSIFY(user.username.empty()); });

  return 0;
}
