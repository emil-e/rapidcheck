#include <catch.hpp>
#include <rapidcheck/catch.h>
#include <rapidcheck/state.h>

#include "util/GenUtils.h"

using namespace rc;
using namespace rc::test;

namespace {

struct Bag {
  std::vector<int> items;
  bool open = false;
};

using BagCommand = state::Command<Bag, Bag>;

struct Open : public BagCommand {
  void preconditions(const Model &s0) const override {
    RC_PRE(!s0.open);
  }

  void apply(Model &s0) const override {
    s0.open = true;
  }

  void run(const Model &s0, Sut &sut) const override { sut.open = true; }

  void show(std::ostream &os) const override { os << "Open"; }
};

struct Close : public BagCommand {
  void preconditions(const Model &s0) const override {
    RC_PRE(s0.open);
  }

  void apply(Model &s0) const override {
    s0.open = false;
  }

  void run(const Model &s0, Sut &sut) const override { sut.open = false; }

  void show(std::ostream &os) const override { os << "Close"; }
};

struct Add : public BagCommand {
  int item = *gen::inRange<int>(0, 10);

  void preconditions(const Model &s0) const override {
    RC_PRE(s0.open);
  }

  void apply(Model &s0) const override {
    s0.items.push_back(item);
  }

  void run(const Model &s0, Sut &sut) const override {
    sut.items.push_back(item);
  }

  void show(std::ostream &os) const override { os << "Add(" << item << ")"; }
};

struct Del : public BagCommand {
  std::size_t index;

  explicit Del(const Bag &s0) {
    index = *gen::inRange<std::size_t>(0, s0.items.size());
  }

  void preconditions(const Model &s0) const override {
    RC_PRE(s0.open);
    RC_PRE(index < s0.items.size());
  }

  void apply(Model &s0) const override {
    auto s1 = s0;
    s0.items.erase(begin(s0.items) + index);
  }

  void run(const Model &s0, Sut &sut) const override {
    sut.items.erase(begin(sut.items) + index);
  }

  void show(std::ostream &os) const override { os << "Del(" << index << ")"; }
};

struct BuggyGet : public BagCommand {
  std::size_t index;

  explicit BuggyGet(const Bag &s0) {
    index = *gen::inRange<std::size_t>(0, s0.items.size());
  }

  void preconditions(const Model &s0) const override {
    RC_PRE(s0.open);
    RC_PRE(index < s0.items.size());
  }

  void run(const Model &s0, Sut &sut) const override {
    RC_ASSERT(sut.items.size() < 2U);
    RC_ASSERT(sut.items[index] == s0.items[index]);
  }

  void show(std::ostream &os) const override {
    os << "BuggyGet(" << index << ")";
  }
};

struct BuggyDelAll : public BagCommand {
  int value;

  explicit BuggyDelAll(const Bag &s0) { value = *gen::elementOf(s0.items); }

  void preconditions(const Model &s0) const override {
    RC_PRE(s0.open);
    RC_PRE(std::find(begin(s0.items), end(s0.items), value) != end(s0.items));
  }

  void apply(Model &s0) const override {
    s0.items.erase(std::remove(begin(s0.items), end(s0.items), value),
                   end(s0.items));
  }

  void run(const Model &s0, Sut &sut) const override { RC_FAIL("Bug!"); }

  void show(std::ostream &os) const override {
    os << "BuggyDelAll(" << value << ")";
  }
};

struct SneakyBuggyGet : public BagCommand {
  int value;

  explicit SneakyBuggyGet(const Bag &s0) { value = *gen::elementOf(s0.items); }

  void preconditions(const Model &s0) const override {
    RC_PRE(s0.open);
    RC_PRE(std::find(begin(s0.items), end(s0.items), value) != end(s0.items));
  }

  void run(const Model &s0, Sut &sut) const override { RC_ASSERT(value != 2); }

  void show(std::ostream &os) const override {
    os << "SneakyBuggyGet(" << value << ")";
  }
};

template <typename Cmd>
std::vector<std::string> showCommands(const state::Commands<Cmd> &commands) {
  std::vector<std::string> cmdStrings;
  cmdStrings.reserve(commands.size());
  for (const auto &cmd : commands) {
    std::ostringstream ss;
    cmd->show(ss);
    cmdStrings.push_back(ss.str());
  }

  return cmdStrings;
}

template <typename Cmd>
state::Commands<Cmd> findMinCommands(const GenParams &params,
                                     const Gen<state::Commands<Cmd>> &gen,
                                     const typename Cmd::Model &s0) {
  return searchGen(params.random,
                   params.size,
                   gen,
                   [=](const state::Commands<Cmd> &cmds) {
                     try {
                       typename Cmd::Sut sut;
                       runAll(cmds, s0, sut);
                     } catch (...) {
                       return true;
                     }
                     return false;
                   });
}

} // namespace

TEST_CASE("state integration tests") {
  prop(
      "should find minimum when some commands might fail to generate while "
      "shrinking",
      [](const GenParams &params) {
        Bag s0;
        const auto gen = state::gen::commands<BagCommand>(
            s0, &state::gen::execOneOf<Open, Close, Add, Del, BuggyGet>);
        const auto commands = findMinCommands(params, gen, s0);
        const auto cmdStrings = showCommands(commands);
        RC_ASSERT(cmdStrings.size() == 4U);
        RC_ASSERT(cmdStrings[0] == "Open");
        RC_ASSERT(cmdStrings[1] == "Add(0)");
        RC_ASSERT(cmdStrings[2] == "Add(0)");
        RC_ASSERT((cmdStrings[3] == "BuggyGet(0)") ||
                  (cmdStrings[3] == "BuggyGet(1)"));
      });

  prop(
      "should find minimum when later commands depend on the shrunk values of "
      "previous commands",
      [](const GenParams &params) {
        Bag s0;
        const auto gen = state::gen::commands<BagCommand>(
            s0, &state::gen::execOneOf<Open, Close, Add, Del, BuggyDelAll>);
        const auto commands = findMinCommands(params, gen, s0);
        const auto cmdStrings = showCommands(commands);
        std::vector<std::string> expected{"Open", "Add(0)", "BuggyDelAll(0)"};
        RC_ASSERT(cmdStrings == expected);
      });

  prop(
      "should find minimum when later commands depend on the specific values "
      "of previous commands",
      [](const GenParams &params) {
        Bag s0;
        const auto gen = state::gen::commands<BagCommand>(
            s0, &state::gen::execOneOf<Open, Close, Add, Del, SneakyBuggyGet>);
        const auto commands = findMinCommands(params, gen, s0);
        const auto cmdStrings = showCommands(commands);
        std::vector<std::string> expected{"Open", "Add(2)", "SneakyBuggyGet(2)"};
        RC_ASSERT(cmdStrings == expected);
      });
}
