#include <catch.hpp>
#include <rapidcheck-catch.h>

#include <typeinfo>
#include <typeindex>

#include "util/ShowTypeTestUtils.h"
#include "util/GenUtils.h"
#include "util/ShrinkableUtils.h"

#include "rapidcheck/seq/Operations.h"

using namespace rc;
using namespace rc::test;
using namespace rc::detail;
using namespace rc::state;
using namespace rc::state::detail;

namespace {

using StringVec = std::vector<std::string>;
using StringVecCmd = Command<StringVec, StringVec>;
using StringVecCmdSP = std::shared_ptr<const StringVecCmd>;
using StringVecCmdsN = Commands<StringVecCmd>;

struct PushBack : public StringVecCmd {
  PushBack()
      : value(*gen::arbitrary<std::string>()) {}
  std::string value;

  StringVec nextState(const StringVec &s0) const override {
    StringVec s1(s0);
    s1.push_back(value);
    return s1;
  }

  void run(const StringVec &s0, StringVec &sut) const override {
    sut.push_back(value);
  }

  void show(std::ostream &os) const override { os << value; }
};

struct PopBack : public StringVecCmd {
  StringVec nextState(const StringVec &s0) const override {
    RC_PRE(!s0.empty());
    StringVec s1(s0);
    s1.pop_back();
    return s1;
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
  StringVec nextState(const StringVec &s0) const override {
    RC_DISCARD("Preconditions never hold");
  }
};

struct ParamsCmd : StringVecCmd {
  Random random;
  int size;
};

struct SomeCommand : StringVecCmd {};

Gen<std::vector<StringVecCmdSP>> pushBackCommands() {
  return gen::container<std::vector<StringVecCmdSP>>(gen::exec(
      []() -> StringVecCmdSP { return std::make_shared<PushBack>(); }));
}

struct A : public StringVecCmd {};
struct B : public StringVecCmd {};
struct C : public StringVecCmd {};

struct DualConstructible : public StringVecCmd {
  DualConstructible();
  DualConstructible(const StringVec &s)
      : state(s) {}
  StringVec state;
};

struct AlwaysDiscard : public StringVecCmd {
  AlwaysDiscard() { RC_DISCARD("Nope"); }
};

} // namespace

TEST_CASE("Command") {
  SECTION("nextState") {
    prop("default implementation returns unmodified state",
        [](const StringVec &state) {
          RC_ASSERT(StringVecCmd().nextState(state) == state);
        });
  }

  SECTION("run") {
    prop("default implementation does not modify SUT",
        [](const StringVec &state, const StringVec &sut) {
          const auto pre = sut;
          auto post = sut;
          StringVecCmd().run(state, post);
          RC_ASSERT(pre == post);
        });
  }

  SECTION("show") {
    SECTION("outputs a string which contains the name of the class") {
      // TODO this is the only restriction I dare to place on this right
      // now until we can maybe get some sanitizing going
      std::ostringstream os;
      SomeCommand().show(os);
      REQUIRE(os.str().find("SomeCommand") != std::string::npos);
    }
  }
}

TEST_CASE("Commands") {
  SECTION("nextState") {
    prop("returns next state by applying the commands in sequence",
        [](const StringVec &s0) {
          StringVecCmdsN cmds;
          cmds.commands = *pushBackCommands();

          StringVec expected(s0);
          for (const auto &cmd : cmds.commands) {
            std::ostringstream os;
            cmd->show(os);
            expected.push_back(os.str());
          }

          RC_ASSERT(cmds.nextState(s0) == expected);
        });
  }

  SECTION("run") {
    prop("runs the commands in sequence",
        [](const StringVec &s0) {
          StringVecCmdsN cmds;
          cmds.commands = *pushBackCommands();

          StringVec expected(s0);
          for (const auto &cmd : cmds.commands) {
            std::ostringstream os;
            cmd->show(os);
            expected.push_back(os.str());
          }

          StringVec actual(s0);
          cmds.run(s0, actual);
          RC_ASSERT(actual == expected);
        });
  }

  SECTION("show") {
    prop("displays each command on a separate line",
        [] {
          StringVecCmdsN cmds;
          cmds.commands = *pushBackCommands();

          std::ostringstream expected;
          for (const auto &cmd : cmds.commands) {
            cmd->show(expected);
            expected << std::endl;
          }

          std::ostringstream actual;
          cmds.show(actual);
          RC_ASSERT(expected.str() == actual.str());
        });
  }
}

TEST_CASE("isValidCommand") {
  prop("returns true for valid commands",
      [] { RC_ASSERT(isValidCommand(PushBack(), StringVec())); });

  prop("returns false for invalid commands",
      [] { RC_ASSERT(!isValidCommand(PopBack(), StringVec())); });
}

Gen<StringVecCmdSP> captureParams(const StringVec &vec) {
  return [](const Random &random, int size) {
    auto paramsCmd = std::make_shared<ParamsCmd>();
    paramsCmd->random = random;
    paramsCmd->size = size;
    return shrinkable::just(
        std::static_pointer_cast<const StringVecCmd>(paramsCmd));
  };
}

std::vector<GenParams> collectParams(const StringVecCmdsN &cmds) {
  std::vector<GenParams> params;
  std::transform(begin(cmds.commands),
      end(cmds.commands),
      std::back_inserter(params),
      [](const StringVecCmdSP &cmd) {
        const auto paramsCmd = std::static_pointer_cast<const ParamsCmd>(cmd);
        GenParams params;
        params.random = paramsCmd->random;
        params.size = paramsCmd->size;
        return params;
      });
  return params;
}

std::set<Random> collectRandoms(const StringVecCmdsN &cmds) {
  const auto params = collectParams(cmds);
  std::set<Random> randoms;
  std::transform(begin(params),
      end(params),
      std::inserter(randoms, randoms.end()),
      [](const GenParams &params) { return params.random; });
  return randoms;
}

using IntVec = std::vector<int>;
using IntVecCmd = Command<IntVec, IntVec>;
using IntVecCmdSP = std::shared_ptr<const IntVecCmd>;
using IntVecCmds = Commands<IntVecCmd>;

struct CountCmd : public IntVecCmd {
  CountCmd(int x)
      : value(x) {}
  int value;

  IntVec nextState(const IntVec &s0) const override {
    RC_PRE(!s0.empty());
    RC_PRE(s0.back() == (value - 1));
    IntVec s1(s0);
    s1.push_back(value);
    return s1;
  }

  void run(const IntVec &s0, IntVec &sut) const override {
    sut.push_back(value);
  }

  void show(std::ostream &os) const { os << value; }
};

TEST_CASE("genCommands") {
  prop("command sequences are always valid",
      [](const GenParams &params, const StringVec &s0) {
        const auto gen =
            genCommands<StringVecCmd>(s0, &anyCommand<PushBack, PopBack>);
        onAnyPath(gen(params.random, params.size),
            [&](const Shrinkable<StringVecCmdsN> &value,
                      const Shrinkable<StringVecCmdsN> &shrink) {
              RC_ASSERT(isValidCommand(value.value(), s0));
            });
      });

  prop("shrinks are shorter or equal length when compared to original",
      [](const GenParams &params, const StringVec &s0) {
        const auto gen =
            genCommands<StringVecCmd>(s0, &anyCommand<PushBack, PopBack>);
        onAnyPath(gen(params.random, params.size),
            [&](const Shrinkable<StringVecCmdsN> &value,
                      const Shrinkable<StringVecCmdsN> &shrink) {
              RC_ASSERT(value.value().commands.size() <=
                  value.value().commands.size());
            });
      });

  prop("passed random generators are unique",
      [](const GenParams &params) {
        const auto gen = genCommands<StringVecCmd>(StringVec(), &captureParams);
        const auto cmds = gen(params.random, params.size).value();
        const auto randoms = collectRandoms(cmds);
        RC_ASSERT(randoms.size() == cmds.commands.size());
      });

  prop("shrinks use a subset of the original random generators",
      [](const GenParams &params) {
        const auto gen = genCommands<StringVecCmd>(StringVec(), &captureParams);
        onAnyPath(gen(params.random, params.size),
            [&](const Shrinkable<StringVecCmdsN> &value,
                      const Shrinkable<StringVecCmdsN> &shrink) {
              const auto valueRandoms = collectRandoms(value.value());
              const auto shrinkRandoms = collectRandoms(shrink.value());
              std::vector<Random> intersection;
              std::set_intersection(begin(valueRandoms),
                  end(valueRandoms),
                  begin(shrinkRandoms),
                  end(shrinkRandoms),
                  std::back_inserter(intersection));
              RC_ASSERT(intersection.size() == shrinkRandoms.size());
            });
      });

  prop("passes the correct size",
      [](const GenParams &params) {
        const auto gen = genCommands<StringVecCmd>(StringVec(), &captureParams);
        onAnyPath(gen(params.random, params.size),
            [&](const Shrinkable<StringVecCmdsN> &value,
                      const Shrinkable<StringVecCmdsN> &shrink) {
              const auto allParams = collectParams(value.value());
              RC_ASSERT(std::all_of(begin(allParams),
                  end(allParams),
                  [&](const GenParams &p) { return p.size == params.size; }));
            });
      });

  prop("correctly threads the state when generating commands",
      [](const GenParams &params) {
        IntVec s0({0});
        const auto gen = genCommands<IntVecCmd>(s0,
            [](const IntVec &vec) {
              auto cmd = std::make_shared<const CountCmd>(vec.back() + 1);
              return gen::just(
                  std::move(std::static_pointer_cast<const IntVecCmd>(cmd)));
            });

        onAnyPath(gen(params.random, params.size),
            [&](const Shrinkable<IntVecCmds> &value,
                      const Shrinkable<IntVecCmds> &shrink) {
              auto sut = s0;
              value.value().run(s0, sut);
              int x = 0;
              for (int value : sut)
                RC_ASSERT(value == x++);
            });
      });

  prop("finds minimum where one commands always fails",
      [](const GenParams &params, const StringVec &s0) {
        const auto gen = genCommands<StringVecCmd>(
            s0, &anyCommand<AlwaysFail, PushBack, PopBack, SomeCommand>);
        const auto result = searchGen(params.random,
            params.size,
            gen,
            [&](const StringVecCmdsN &cmds) {
              try {
                StringVec sut = s0;
                cmds.run(s0, sut);
              } catch (...) {
                return true;
              }
              return false;

            });

        RC_ASSERT(result.commands.size() == 1);
        std::ostringstream os;
        result.commands.front()->show(os);
        RC_ASSERT(os.str().find("AlwaysFail") != std::string::npos);
      });

  // TODO test give up
}

TEST_CASE("anyCommand") {
  prop("returns one of the commands",
      [](const GenParams &params, const StringVec &s0) {
        const auto cmd =
            anyCommand<A, B, C>(s0)(params.random, params.size).value();
        const auto &cmdRef = *cmd;
        const auto &id = typeid(cmdRef);
        RC_ASSERT((id == typeid(A)) || (id == typeid(B)) || (id == typeid(C)));
      });

  prop("all commands are eventually returned",
      [](const GenParams &params, const StringVec &s0) {
        auto r = params.random;
        const auto gen = anyCommand<A, B, C>(s0);
        std::set<std::type_index> all{typeid(A), typeid(B), typeid(C)};
        std::set<std::type_index> generated;
        while (generated != all) {
          const auto cmd = gen(r.split(), params.size).value();
          const auto &cmdRef = *cmd;
          generated.emplace(typeid(cmdRef));
        }
        RC_SUCCEED("All generated");
      });

  prop("uses state constructor if there is one, passing it the state",
      [](const GenParams &params, const StringVec &s0) {
        const auto cmd =
            anyCommand<DualConstructible>(s0)(params.random, params.size)
                .value();
        RC_ASSERT(static_cast<const DualConstructible &>(*cmd).state == s0);
      });
}

TEST_CASE("show(Command)") {
  prop("passing a generic command to show yields the same as Command::show",
      [] {
        PushBack cmd;
        std::ostringstream expected;
        cmd.show(expected);
        std::ostringstream actual;
        show(cmd, actual);
        RC_ASSERT(actual.str() == expected.str());
      });

  prop("passing Commands to show yields the same result as Commands::show",
      [] {
        StringVecCmdsN cmds;
        cmds.commands = *pushBackCommands();
        std::ostringstream expected;
        cmds.show(expected);
        std::ostringstream actual;
        show(cmds, actual);
        RC_ASSERT(actual.str() == expected.str());
      });
}

TEST_CASE("typeToString<Command<...>>") {
  using CommandT = Command<Foo, Bar>;
  REQUIRE(typeToString<CommandT>() == "Command<FFoo, BBar>");
}

TEST_CASE("typeToString<Commands<...>>") {
  using CommandsT = Commands<Foo>;
  REQUIRE(typeToString<CommandsT>() == "Commands<FFoo>");
}

TEST_CASE("state::check") {
  prop("if no command fails, check succeeds",
      [](const StringVec &s0, StringVec sut) {
        state::check(s0, sut, &anyCommand<PushBack>);
      });

  prop("if some command fails, check fails",
      [](const StringVec &s0, StringVec sut) {
        try {
          state::check(s0, sut, &anyCommand<AlwaysFail>);
          RC_FAIL("Check succeeded");
        } catch (const CaseResult &result) {
          RC_ASSERT(result.type == CaseResult::Type::Failure);
        }
      });
}
