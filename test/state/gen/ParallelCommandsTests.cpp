#include <catch.hpp>
#include <rapidcheck/catch.h>
#include <rapidcheck/state.h>

#include "util/GenUtils.h"
#include "util/StringVec.h"
#include "util/ShrinkableUtils.h"

using namespace rc;
using namespace rc::test;
using namespace rc::state;
using namespace rc::state::detail;

namespace {

using StringVecParCmds = ParallelCommands<StringVecCmd>;

template <typename Cmd>
std::size_t size(const ParallelCommands<Cmd> &cmds) {
  return cmds.prefix.size() + cmds.left.size() + cmds.right.size();
}

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

std::vector<GenParams> collectParams(const StringVecParCmds &cmds) {
  std::vector<GenParams> params;

  const auto prefix = collectParams(cmds.prefix);
  params.insert(params.end(), prefix.begin(), prefix.end());

  const auto left = collectParams(cmds.left);
  params.insert(params.end(), left.begin(), left.end());

  const auto right = collectParams(cmds.right);
  params.insert(params.end(), right.begin(), right.end());

  return params;
}

std::set<Random> collectRandoms(const StringVecParCmds &cmds) {
  const auto params = collectParams(cmds);
  std::set<Random> randoms;
  std::transform(begin(params),
                 end(params),
                 std::inserter(randoms, randoms.end()),
                 [](const GenParams &params) { return params.random; });
  return randoms;
}

} // namespace

TEST_CASE("state::gen::parallelcommands") {
  prop("shrinks are shorter or equal length when compared to original",
       [](const GenParams &params, const StringVec &s0) {
         const auto gen = state::gen::parallelCommands<StringVecCmd>(
             s0, &state::gen::execOneOf<PushBack, PopBack>);
         onAnyPath(gen(params.random, params.size),
                   [&](const Shrinkable<StringVecParCmds> &value,
                       const Shrinkable<StringVecParCmds> &shrink) {
                     RC_ASSERT(size(value.value()) <= size(value.value()));
                   });
       });

  prop("passed random generators are unique",
       [](const GenParams &params) {
         const auto gen = state::gen::parallelCommands<StringVecCmd>(
             StringVec(), &captureParams);
         const auto cmds = gen(params.random, params.size).value();
         const auto randoms = collectRandoms(cmds);
         RC_ASSERT(randoms.size() == size(cmds));
       });

  prop("shrinks use a subset of the original random generators",
       [](const GenParams &params) {
         const auto gen = state::gen::parallelCommands<StringVecCmd>(
             StringVec(), &captureParams);
         onAnyPath(gen(params.random, params.size),
                   [&](const Shrinkable<StringVecParCmds> &value,
                       const Shrinkable<StringVecParCmds> &shrink) {
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
         const auto gen = state::gen::parallelCommands<StringVecCmd>(
             StringVec(), &captureParams);
         onAnyPath(gen(params.random, params.size),
                   [&](const Shrinkable<StringVecParCmds> &value,
                       const Shrinkable<StringVecParCmds> &shrink) {
                     const auto allParams = collectParams(value.value());
                     RC_ASSERT(std::all_of(begin(allParams),
                                           end(allParams),
                                           [&](const GenParams &p) {
                                             return p.size == params.size;
                                           }));
                   });
       });

  prop("count never exceeds size",
       [](const GenParams &params) {
         const auto gen = state::gen::parallelCommands<StringVecCmd>(
             StringVec(), &captureParams);
         onAnyPath(gen(params.random, params.size),
                   [&](const Shrinkable<StringVecParCmds> &value,
                       const Shrinkable<StringVecParCmds> &shrink) {
                     RC_ASSERT(size(value.value()) <= params.size);
                   });
       });
}
