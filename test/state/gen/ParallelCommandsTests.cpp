#include <catch.hpp>
#include <rapidcheck/catch.h>
#include <rapidcheck/state.h>

#include "util/GenUtils.h"
#include "util/StringVec.h"

using namespace rc;
using namespace rc::test;
using namespace rc::state::gen::detail;

namespace {

Gen<std::vector<StringVecCmdSP>> pushBackCommands() {
  return gen::container<std::vector<StringVecCmdSP>>(gen::exec(
      []() -> StringVecCmdSP { return std::make_shared<PushBack>(); }));
}

} // namespace

// TEST_CASE("state::splitCommands") {
//   prop("splits commands into three equally(ish) sized groups",
//        [] {
//          const auto cmds = *pushBackCommands();
//          auto parallelCmdSeq = toParallelSequence(cmds);

//          // Merged sequences should equal original sequence
//          auto mergedCmds = parallelCmdSeq.serialCmdSeq;
//          mergedCmds.insert(mergedCmds.end(),
//                            parallelCmdSeq.parallelCmdSeq1.begin(),
//                            parallelCmdSeq.parallelCmdSeq1.end());
//          mergedCmds.insert(mergedCmds.end(),
//                            parallelCmdSeq.parallelCmdSeq2.begin(),
//                            parallelCmdSeq.parallelCmdSeq2.end());

//          RC_ASSERT(std::equal(cmds.begin(), cmds.end(), mergedCmds.begin()));
//        });
//}
