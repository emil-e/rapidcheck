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

  void run(const StringVec &s0, StringVec &sut) const override {
    sut.push_back(value);
  }

  void show(std::ostream &os) const override { os << value; }
};

struct PopBack : public StringVecCmd {
  void apply(StringVec &s0) const override {
    RC_PRE(!s0.empty());
    s0.pop_back();
  }

  void run(const StringVec &s0, StringVec &sut) const override {
    sut.pop_back();
  }
};

struct AlwaysFail : public StringVecCmd {
  void run(const StringVec &s0, StringVec &sut) const override {
    RC_FAIL("Always fails");
  }
};

struct PreNeverHolds : public StringVecCmd {
  void apply(StringVec &s0) const override {
    RC_DISCARD("Preconditions never hold");
  }

  void run(const StringVec &s0, StringVec &sut) const override {}
};

struct SomeCommand : StringVecCmd {
  void run(const StringVec &s0, StringVec &sut) const override {}
};


struct StringVecPar {
  StringVecPar(std::vector<std::string> strings,
               std::shared_ptr<std::mutex> mutex)
      : strings(strings)
      , mutex(std::move(mutex)) {}

  std::vector<std::string> strings;
  std::shared_ptr<std::mutex> mutex;
};

using StringVecParCmd = state::Command<StringVec, StringVecPar>;
using StringVecParCmdSP = std::shared_ptr<const StringVecParCmd>;

struct PushBackPar : public StringVecParCmd {
  std::string value;

  PushBackPar()
      : value(*gen::arbitrary<std::string>()) {}

  PushBackPar(std::string str)
      : value(std::move(str)) {}

  void apply(StringVec &s0) const override { s0.push_back(value); }

  std::function<void(const StringVec &)> run(StringVecPar &sut) const override {
    std::lock_guard<std::mutex> lock(*sut.mutex);
    sut.strings.push_back(value);
    return [](const StringVec &) {};
  }

  void show(std::ostream &os) const override { os << value; }
};

} // namespace test
} // namespace rc
