#include <catch.hpp>
#include <rapidcheck/catch.h>
#include <rapidcheck/state.h>

#include "util/GenUtils.h"
#include "util/IntVec.h"
#include "util/NonCopyableModel.h"
#include "util/ShrinkableUtils.h"

using namespace rc;
using namespace rc::test;

namespace {

struct ParamsCmd : IntVecCmd {
  Random random;
  int size;
};

Gen<IntVecCmdSP> captureParams(const IntVec &vec) {
  return [](const Random &random, int size) {
    auto paramsCmd = std::make_shared<ParamsCmd>();
    paramsCmd->random = random;
    paramsCmd->size = size;
    return shrinkable::just(
        std::static_pointer_cast<const IntVecCmd>(paramsCmd));
  };
}

std::vector<GenParams> collectParams(const IntVecCmds &cmds) {
  std::vector<GenParams> params;
  std::transform(begin(cmds),
                 end(cmds),
                 std::back_inserter(params),
                 [](const IntVecCmdSP &cmd) {
                   const auto paramsCmd =
                       std::static_pointer_cast<const ParamsCmd>(cmd);
                   GenParams params;
                   params.random = paramsCmd->random;
                   params.size = paramsCmd->size;
                   return params;
                 });
  return params;
}

std::set<Random> collectRandoms(const IntVecCmds &cmds) {
  const auto params = collectParams(cmds);
  std::set<Random> randoms;
  std::transform(begin(params),
                 end(params),
                 std::inserter(randoms, randoms.end()),
                 [](const GenParams &params) { return params.random; });
  return randoms;
}

struct CountCmd : public IntVecCmd {
  CountCmd(int x)
      : value(x) {}
  int value;

  void preconditions(const IntVec &s0) const override {
    RC_PRE(s0.back() == (value - 1));
  }

  void apply(IntVec &s0) const override { s0.push_back(value); }

  void run(const IntVec &s0, IntVec &sut) const override {
    sut.push_back(value);
  }

  void show(std::ostream &os) const override { os << value; }
};

struct TaggedCountdownCmd : public state::Command<bool, bool> {
  int countdown = *genCountdown();
  int tag = *gen::noShrink(gen::arbitrary<int>());
};

struct DeprecatedCmd : public IntVecCmd {
  void apply(IntVec &s0) const override { RC_DISCARD(); }
};

} // namespace

TEST_CASE("state::gen::commands") {
  prop("command sequences are always valid",
       [](const GenParams &params, const IntVec &s0) {
         const auto gen = state::gen::commands<IntVecCmd>(
             s0, state::gen::execOneOfWithArgs<PushBack, PopBack>());
         onAnyPath(gen(params.random, params.size),
                   [&](const Shrinkable<IntVecCmds> &value,
                       const Shrinkable<IntVecCmds> &shrink) {
                     RC_ASSERT(isValidSequence(value.value(), s0));
                   });
       });

  prop("shrinks are shorter or equal length when compared to original",
       [](const GenParams &params, const IntVec &s0) {
         const auto gen = state::gen::commands<IntVecCmd>(
             s0, state::gen::execOneOfWithArgs<PushBack, PopBack>());
         onAnyPath(gen(params.random, params.size),
                   [&](const Shrinkable<IntVecCmds> &value,
                       const Shrinkable<IntVecCmds> &shrink) {
                     RC_ASSERT(value.value().size() <= value.value().size());
                   });
       });

  prop("passed random generators are unique",
       [](const GenParams &params) {
         const auto gen =
             state::gen::commands<IntVecCmd>(IntVec(), &captureParams);
         const auto cmds = gen(params.random, params.size).value();
         const auto randoms = collectRandoms(cmds);
         RC_ASSERT(randoms.size() == cmds.size());
       });

  prop("shrinks use a subset of the original random generators",
       [](const GenParams &params) {
         const auto gen =
             state::gen::commands<IntVecCmd>(IntVec(), &captureParams);
         onAnyPath(gen(params.random, params.size),
                   [&](const Shrinkable<IntVecCmds> &value,
                       const Shrinkable<IntVecCmds> &shrink) {
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
         const auto gen =
             state::gen::commands<IntVecCmd>(IntVec(), &captureParams);
         onAnyPath(gen(params.random, params.size),
                   [&](const Shrinkable<IntVecCmds> &value,
                       const Shrinkable<IntVecCmds> &shrink) {
                     const auto allParams = collectParams(value.value());
                     RC_ASSERT(std::all_of(begin(allParams),
                                           end(allParams),
                                           [&](const GenParams &p) {
                                             return p.size == params.size;
                                           }));
                   });
       });

  prop("correctly threads the state when generating commands",
       [](const GenParams &params) {
         IntVec s0({0});
         const auto gen = state::gen::commands<IntVecCmd>(
             s0,
             [](const IntVec &vec) {
               auto cmd = std::make_shared<const CountCmd>(vec.back() + 1);
               return gen::just(std::static_pointer_cast<const IntVecCmd>(cmd));
             });

         onAnyPath(gen(params.random, params.size),
                   [&](const Shrinkable<IntVecCmds> &value,
                       const Shrinkable<IntVecCmds> &shrink) {
                     auto sut = s0;
                     runAll(value.value(), s0, sut);
                     int x = 0;
                     for (int value : sut) {
                       RC_ASSERT(value == x++);
                     }
                   });
       });

  prop("finds minimum where one commands always fails",
       [](const GenParams &params, const IntVec &s0) {
         const auto gen = state::gen::commands<IntVecCmd>(
             s0,
             state::gen::execOneOfWithArgs<AlwaysFail,
                                           PushBack,
                                           PopBack,
                                           SomeCommand>());
         const auto result = searchGen(params.random,
                                       params.size,
                                       gen,
                                       [&](const IntVecCmds &cmds) {
                                         try {
                                           IntVec sut = s0;
                                           runAll(cmds, s0, sut);
                                         } catch (...) {
                                           return true;
                                         }
                                         return false;

                                       });

         RC_ASSERT(result.size() == 1U);
         std::ostringstream os;
         result.front()->show(os);
         RC_ASSERT(os.str().find("AlwaysFail") != std::string::npos);
       });

  prop(
      "for every shrink for every command, there exists a shrink of the "
      "sequence that includes that shrink",
      [](const GenParams &params) {
        using CommandType = TaggedCountdownCmd::CommandType;
        using CommandsType = state::Commands<CommandType>;

        const auto gen = state::gen::commands<CommandType>(
            false, state::gen::execOneOfWithArgs<TaggedCountdownCmd>());
        const auto shrinkable = gen(params.random, params.size);

        // Pick one of the commands
        const auto commands = shrinkable.value();
        const auto cmdIndex = *gen::inRange<std::size_t>(0, commands.size());
        const auto &command =
            static_cast<const TaggedCountdownCmd &>(*commands[cmdIndex]);

        // Expect to find a command with the same tag but smaller countdown.
        const auto expectedCountdown = *gen::inRange(0, command.countdown);
        RC_ASSERT(seq::any(shrinkable.shrinks(),
                           [&](const Shrinkable<CommandsType> &shrink) {
                             for (const auto &cmd : shrink.value()) {
                               const auto tccmd =
                                   static_cast<const TaggedCountdownCmd &>(
                                       *cmd);
                               if ((tccmd.tag == command.tag) &&
                                   (tccmd.countdown == expectedCountdown)) {
                                 return true;
                               }
                             }

                             return false;
                           }));
      });

  prop(
      "for every command, there exists a shrink of the sequence that does not "
      "include that command",
      [](const GenParams &params) {
        using CommandType = TaggedCountdownCmd::CommandType;
        using CommandsType = state::Commands<CommandType>;

        const auto gen = state::gen::commands<CommandType>(
            false, state::gen::execOneOfWithArgs<TaggedCountdownCmd>());
        const auto shrinkable = gen(params.random, params.size);

        // Pick one of the commands
        const auto commands = shrinkable.value();
        const auto cmdIndex = *gen::inRange<std::size_t>(0, commands.size());
        const auto &command =
            static_cast<const TaggedCountdownCmd &>(*commands[cmdIndex]);

        // Expect to find a shrink that has no command with that tag.
        RC_ASSERT(seq::any(shrinkable.shrinks(),
                           [&](const Shrinkable<CommandsType> &shrink) {
                             for (const auto &cmd : shrink.value()) {
                               const auto tag =
                                   static_cast<const TaggedCountdownCmd &>(*cmd)
                                       .tag;
                               if (tag == command.tag) {
                                 return false;
                               }
                             }

                             return true;
                           }));
      });

  prop("gives up if unable to generate sequence of enough length",
       [](const GenParams &params, const IntVec &s0) {
         const auto gen = state::gen::commands<IntVecCmd>(
             s0, state::gen::execOneOfWithArgs<PreNeverHolds>());

         const auto shrinkable = gen(params.random, params.size);
         RC_ASSERT_THROWS_AS(shrinkable.value(), GenerationFailure);
       });

  prop("discards commands that discard in constructor",
       [](const GenParams &params, const IntVec &s0) {
         const auto gen = state::gen::commands<IntVecCmd>(
             s0,
             state::gen::execOneOfWithArgs<DiscardInConstructor, PushBack>());
         const auto commands = gen(params.random, params.size).value();
         for (const auto &cmd : commands) {
           std::ostringstream ss;
           cmd->show(ss);
           RC_ASSERT(ss.str() != "DiscardInConstructor");
         }
       });

  prop("works with non-copyable models",
       [](const GenParams &params) {
         const auto commands = *genNonCopyableCommands();
         RC_ASSERT(isValidSequence(commands, &initialNonCopyableModel));

         NonCopyableModel s0;
         state::applyAll(commands, s0);
       });

  prop("throws GenerationFailure if command discards in apply(...)",
       [](const GenParams &params, const IntVec &s0) {
         RC_PRE(params.size > 0);
         const auto gen = state::gen::commands<IntVecCmd>(
             s0, state::gen::execOneOfWithArgs<DeprecatedCmd>());
         RC_ASSERT_THROWS_AS(gen(params.random, params.size).value(),
                             GenerationFailure);
       });
}
