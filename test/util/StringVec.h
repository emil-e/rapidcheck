#pragma once

#include <rapidcheck/state.h>

namespace rc {
namespace test {

using StringVec = std::vector<std::string>;
using StringVecCmd = state::Command<StringVec, StringVec>;
using StringVecCmdSP = std::shared_ptr<const StringVecCmd>;
using StringVecCmds = state::Commands<StringVecCmd>;

struct PushBack : public StringVecCmd {
  std::string value;

  PushBack()
      : value(*gen::arbitrary<std::string>()) {}

  PushBack(std::string str)
      : value(std::move(str)) {}

  void apply(StringVec &s0) const override { s0.push_back(value); }

  void run(StringVec &sut) const override {
    sut.push_back(value);
  }

  void show(std::ostream &os) const override { os << value; }
};

struct PopBack : public StringVecCmd {
  void apply(StringVec &s0) const override {
    RC_PRE(!s0.empty());
    s0.pop_back();
  }

  void run(StringVec &sut) const override {
    sut.pop_back();
  }
};

struct AlwaysFail : public StringVecCmd {
  void run(StringVec &sut) const override {
    RC_FAIL("Always fails");
  }
};

struct PreNeverHolds : public StringVecCmd {
  void apply(StringVec &s0) const override {
    RC_DISCARD("Preconditions never hold");
  }
};

struct SomeCommand : StringVecCmd {};

} // namespace test
} // namespace rc
