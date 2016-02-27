#pragma once

#include <rapidcheck/state.h>

namespace rc {
namespace test {

using IntVec = std::vector<int>;
using IntVecCmd = state::Command<IntVec, IntVec>;
using IntVecCmdSP = std::shared_ptr<const IntVecCmd>;
using IntVecCmds = state::Commands<IntVecCmd>;

struct PushBack : public IntVecCmd {
  int value;

  PushBack()
      : value(*gen::arbitrary<int>()) {}

  PushBack(int v)
      : value(v) {}

  void apply(IntVec &s0) const override { s0.push_back(value); }

  void run(const IntVec &s0, IntVec &sut) const override {
    sut.push_back(value);
  }

  void show(std::ostream &os) const override { os << value; }
};

struct PopBack : public IntVecCmd {
  void preconditions(const IntVec &s0) const override {
    RC_PRE(!s0.empty());
  }

  void apply(IntVec &s0) const override {
    s0.pop_back();
  }

  void run(const IntVec &s0, IntVec &sut) const override {
    sut.pop_back();
  }
};

struct AlwaysFail : public IntVecCmd {
  void run(const IntVec &s0, IntVec &sut) const override {
    RC_FAIL("Always fails");
  }
};

struct PreNeverHolds : public IntVecCmd {
  void preconditions(const IntVec &s0) const override {
    RC_DISCARD("Preconditions never hold");
  }
};

struct SomeCommand : IntVecCmd {};

} // namespace test
} // namespace rc
