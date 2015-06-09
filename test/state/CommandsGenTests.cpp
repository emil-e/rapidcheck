#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/StringVec.h"
#include "util/GenUtils.h"
#include "util/ShrinkableUtils.h"

using namespace rc;
using namespace rc::test;
using namespace rc::state;
using namespace rc::state::detail;

namespace {

struct ParamsCmd : StringVecCmd {
  Random random;
  int size;
};

Gen<StringVecCmdSP> captureParams(const StringVec &vec) {
  return [](const Random &random, int size) {
    auto paramsCmd = std::make_shared<ParamsCmd>();
    paramsCmd->random = random;
    paramsCmd->size = size;
    return shrinkable::just(
        std::static_pointer_cast<const StringVecCmd>(paramsCmd));
  };
}

std::vector<GenParams> collectParams(const StringVecCmds &cmds) {
  std::vector<GenParams> params;
  std::transform(begin(cmds),
                 end(cmds),
                 std::back_inserter(params),
                 [](const StringVecCmdSP &cmd) {
                   const auto paramsCmd =
                       std::static_pointer_cast<const ParamsCmd>(cmd);
                   GenParams params;
                   params.random = paramsCmd->random;
                   params.size = paramsCmd->size;
                   return params;
                 });
  return params;
}

std::set<Random> collectRandoms(const StringVecCmds &cmds) {
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
using IntVecCmds = Commands<IntVecCmd>;

struct CountCmd : public IntVecCmd {
  CountCmd(int x)
      : value(x) {}
  int value;

  void apply(IntVec &s0) const override {
    s0.push_back(value);
  }

  void run(const IntVec &s0, IntVec &sut) const override {
    sut.push_back(value);
  }

  void show(std::ostream &os) const { os << value; }
};

} // namespace

TEST_CASE("genCommands") {
  prop("command sequences are always valid",
       [](const GenParams &params, const StringVec &s0) {
         const auto gen =
             genCommands<StringVecCmd>(s0, &anyCommand<PushBack, PopBack>);
         onAnyPath(gen(params.random, params.size),
                   [&](const Shrinkable<StringVecCmds> &value,
                       const Shrinkable<StringVecCmds> &shrink) {
                     RC_ASSERT(isValidSequence(value.value(), s0));
                   });
       });

  prop("shrinks are shorter or equal length when compared to original",
       [](const GenParams &params, const StringVec &s0) {
         const auto gen =
             genCommands<StringVecCmd>(s0, &anyCommand<PushBack, PopBack>);
         onAnyPath(gen(params.random, params.size),
                   [&](const Shrinkable<StringVecCmds> &value,
                       const Shrinkable<StringVecCmds> &shrink) {
                     RC_ASSERT(value.value().size() <= value.value().size());
                   });
       });

  prop("passed random generators are unique",
       [](const GenParams &params) {
         const auto gen =
             genCommands<StringVecCmd>(StringVec(), &captureParams);
         const auto cmds = gen(params.random, params.size).value();
         const auto randoms = collectRandoms(cmds);
         RC_ASSERT(randoms.size() == cmds.size());
       });

  prop("shrinks use a subset of the original random generators",
       [](const GenParams &params) {
         const auto gen =
             genCommands<StringVecCmd>(StringVec(), &captureParams);
         onAnyPath(gen(params.random, params.size),
                   [&](const Shrinkable<StringVecCmds> &value,
                       const Shrinkable<StringVecCmds> &shrink) {
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
             genCommands<StringVecCmd>(StringVec(), &captureParams);
         onAnyPath(gen(params.random, params.size),
                   [&](const Shrinkable<StringVecCmds> &value,
                       const Shrinkable<StringVecCmds> &shrink) {
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
         const auto gen = genCommands<IntVecCmd>(
             s0,
             [](const IntVec &vec) {
               auto cmd = std::make_shared<const CountCmd>(vec.back() + 1);
               return gen::just(
                   std::move(std::static_pointer_cast<const IntVecCmd>(cmd)));
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
       [](const GenParams &params, const StringVec &s0) {
         const auto gen = genCommands<StringVecCmd>(
             s0, &anyCommand<AlwaysFail, PushBack, PopBack, SomeCommand>);
         const auto result = searchGen(params.random,
                                       params.size,
                                       gen,
                                       [&](const StringVecCmds &cmds) {
                                         try {
                                           StringVec sut = s0;
                                           runAll(cmds, s0, sut);
                                         } catch (...) {
                                           return true;
                                         }
                                         return false;

                                       });

         RC_ASSERT(result.size() == 1);
         std::ostringstream os;
         result.front()->show(os);
         RC_ASSERT(os.str().find("AlwaysFail") != std::string::npos);
       });

  // TODO test give up
}
