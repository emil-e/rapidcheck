#include <catch.hpp>
#include <rapidcheck-catch.h>

#include <stack>

using namespace rc;
using namespace rc::detail;

namespace {

// Our test params

struct ParamA {
  typedef std::string ValueType;
  static std::string defaultValue() { return "default"; }
};

struct ParamB {
  typedef int ValueType;
  static int defaultValue() { return 1337; }
};

struct ImplicitScopeModel {
  ImplicitScopeModel() {
    bindingsA.push(ParamA::defaultValue());
    bindingsB.push(ParamB::defaultValue());
  }

  std::stack<ParamA::ValueType> bindingsA;
  std::stack<ParamB::ValueType> bindingsB;
};

typedef std::stack<ImplicitScopeModel> ImplicitParamModel;

struct ImplicitParamSystem {
  std::stack<ImplicitParam<ParamA>> bindingsA;
  std::stack<ImplicitParam<ParamB>> bindingsB;
  std::stack<ImplicitScope> scopes;
};

typedef state::Command<ImplicitParamModel, ImplicitParamSystem> ImplicitCommand;
typedef std::shared_ptr<ImplicitCommand> ImplicitCommandSP;

struct NewScope : public ImplicitCommand {
  ImplicitParamModel nextState(const ImplicitParamModel &s0) const override {
    auto s1(s0);
    s1.emplace();
    return s1;
  }

  void run(const ImplicitParamModel &s0,
      ImplicitParamSystem &system) const override {
    system.scopes.emplace();
    RC_ASSERT(ImplicitParam<ParamA>::value() == ParamA::defaultValue());
    RC_ASSERT(ImplicitParam<ParamB>::value() == ParamB::defaultValue());
  }
};

struct BindA : public ImplicitCommand {
  ParamA::ValueType value = *gen::arbitrary<ParamA::ValueType>();

  ImplicitParamModel nextState(const ImplicitParamModel &s0) const override {
    RC_PRE(!s0.empty());

    auto s1(s0);
    s1.top().bindingsA.emplace(value);
    return s1;
  }

  void run(const ImplicitParamModel &s0,
      ImplicitParamSystem &system) const override {
    system.bindingsA.emplace(value);
    RC_ASSERT(ImplicitParam<ParamA>::value() == value);
  }

  void show(std::ostream &os) const override { os << "BindA: " << value; }
};

struct BindB : public ImplicitCommand {
  ParamB::ValueType value = *gen::arbitrary<ParamB::ValueType>();

  ImplicitParamModel nextState(const ImplicitParamModel &s0) const override {
    RC_PRE(!s0.empty());

    auto s1(s0);
    s1.top().bindingsB.emplace(value);
    return s1;
  }

  void run(const ImplicitParamModel &s0,
      ImplicitParamSystem &system) const override {
    system.bindingsB.emplace(value);
    RC_ASSERT(ImplicitParam<ParamB>::value() == value);
  }

  void show(std::ostream &os) const override { os << "BindB: " << value; }
};

struct ModifyA : public ImplicitCommand {
  ParamA::ValueType value = *gen::arbitrary<ParamA::ValueType>();

  ImplicitParamModel nextState(const ImplicitParamModel &s0) const override {
    RC_PRE(!s0.empty());

    auto s1(s0);
    s1.top().bindingsA.top() = value;
    return s1;
  }

  void run(const ImplicitParamModel &s0,
      ImplicitParamSystem &system) const override {
    ImplicitParam<ParamA>::value() = value;
    RC_ASSERT(ImplicitParam<ParamA>::value() == value);
  }

  void show(std::ostream &os) const override { os << "ModifyA: " << value; }
};

struct ModifyB : public ImplicitCommand {
  ParamB::ValueType value = *gen::arbitrary<ParamB::ValueType>();

  ImplicitParamModel nextState(const ImplicitParamModel &s0) const override {
    RC_PRE(!s0.empty());

    auto s1(s0);
    s1.top().bindingsB.top() = value;
    return s1;
  }

  void run(const ImplicitParamModel &s0,
      ImplicitParamSystem &system) const override {
    ImplicitParam<ParamB>::value() = value;
    RC_ASSERT(ImplicitParam<ParamB>::value() == value);
  }

  void show(std::ostream &os) const override { os << "ModifyB: " << value; }
};

struct PopA : public ImplicitCommand {
  ImplicitParamModel nextState(const ImplicitParamModel &s0) const override {
    RC_PRE(!s0.empty());
    RC_PRE(s0.top().bindingsA.size() > 1);

    auto s1(s0);
    s1.top().bindingsA.pop();
    return s1;
  }

  void run(const ImplicitParamModel &s0,
      ImplicitParamSystem &system) const override {
    system.bindingsA.pop();
    auto expected = nextState(s0).top().bindingsA.top();
    RC_ASSERT(ImplicitParam<ParamA>::value() == expected);
  }
};

struct PopB : public ImplicitCommand {
  ImplicitParamModel nextState(const ImplicitParamModel &s0) const override {
    RC_PRE(!s0.empty());
    RC_PRE(s0.top().bindingsB.size() > 1);

    auto s1(s0);
    s1.top().bindingsB.pop();
    return s1;
  }

  void run(const ImplicitParamModel &s0,
      ImplicitParamSystem &system) const override {
    system.bindingsB.pop();
    auto expected = nextState(s0).top().bindingsB.top();
    RC_ASSERT(ImplicitParam<ParamB>::value() == expected);
  }
};

struct PopScope : public ImplicitCommand {
  ImplicitParamModel nextState(const ImplicitParamModel &s0) const override {
    // Never pop last scope
    RC_PRE(s0.size() > 1);

    auto s1(s0);
    s1.pop();
    return s1;
  }

  void run(const ImplicitParamModel &s0,
      ImplicitParamSystem &system) const override {
    auto s1(s0);
    while (s1.top().bindingsA.size() > 1) {
      s1.top().bindingsA.pop();
      system.bindingsA.pop();
    }
    while (s1.top().bindingsB.size() > 1) {
      s1.top().bindingsB.pop();
      system.bindingsB.pop();
    }
    s1.pop();
    system.scopes.pop();
    RC_ASSERT(ImplicitParam<ParamA>::value() == s1.top().bindingsA.top());
    RC_ASSERT(ImplicitParam<ParamB>::value() == s1.top().bindingsB.top());
  }
};

} // namespace

TEST_CASE("ImplicitParam") {
  prop("stateful test",
      [] {
        ImplicitParamModel s0;
        ImplicitParamSystem sut;
        state::check(s0,
            sut,
            state::anyCommand<NewScope,
                         BindA,
                         BindB,
                         ModifyA,
                         ModifyB,
                         PopA,
                         PopB,
                         PopScope>);
      });

  SECTION("unit tests") {
    ImplicitScope scope;

    ImplicitParam<ParamA> a1("foobar");
    ImplicitParam<ParamB> b1(123);

    SECTION("with new binding, value() return the values of that binding") {
      REQUIRE(ImplicitParam<ParamA>::value() == "foobar");
      REQUIRE(ImplicitParam<ParamB>::value() == 123);
    }

    SECTION("allows modifications of bound value") {
      ImplicitParam<ParamA>::value() = "barfoo";
      ImplicitParam<ParamB>::value() = 321;
      REQUIRE(ImplicitParam<ParamA>::value() == "barfoo");
      REQUIRE(ImplicitParam<ParamB>::value() == 321);
    }

    { // begin scope2
      ImplicitScope scope2;
      SECTION("value() yields default value in fresh scope") {
        REQUIRE(ImplicitParam<ParamA>::value() == ParamA::defaultValue());
        REQUIRE(ImplicitParam<ParamB>::value() == ParamB::defaultValue());
      }

      SECTION("allows modification of default values") {
        ImplicitParam<ParamA>::value() = "ohyeah";
        ImplicitParam<ParamB>::value() = 2014;
        REQUIRE(ImplicitParam<ParamA>::value() == "ohyeah");
        REQUIRE(ImplicitParam<ParamB>::value() == 2014);
      }

      SECTION(
          "with new scope and binding, value() the value of that"
          " binding") {
        ImplicitParam<ParamA> a1("bind!");
        ImplicitParam<ParamB> b1(101);
        REQUIRE(ImplicitParam<ParamA>::value() == "bind!");
        REQUIRE(ImplicitParam<ParamB>::value() == 101);
      }
    } // end scope2

    SECTION("when scope goes out of scope, previous bindings return") {
      REQUIRE(ImplicitParam<ParamA>::value() == "foobar");
      REQUIRE(ImplicitParam<ParamB>::value() == 123);
    }
  }
}
