#pragma once

#include "rapidcheck/state/gen/Commands.h"

namespace rc {
namespace state {
namespace gen {
namespace detail {

template<typename Cmds>
struct ParallelCommandSequence
{
  ParallelCommandSequence(const Cmds& serialCmdSeq,
                          const Cmds& parallelCmdSeq1,
                          const Cmds& parallelCmdSeq2)
    : serialCmdSeq(serialCmdSeq)
    , parallelCmdSeq1(parallelCmdSeq1)
    , parallelCmdSeq2(parallelCmdSeq2)
  {
  }

  Cmds serialCmdSeq;
  Cmds parallelCmdSeq1;
  Cmds parallelCmdSeq2;
};

template<typename Cmds>
ParallelCommandSequence<Cmds> toParallelSequence(const Cmds& commands)
{
  // TODO: Refactor to a single case.
  auto cmdCount = commands.size();
  // If there are fewer than 12 commands, make all parallel
  if (cmdCount < 12) {
    auto groupSize = cmdCount / 2;
    auto p1Begin = commands.begin();
    auto p2Begin = p1Begin + groupSize;
    return ParallelCommandSequence<Cmds>(
          Cmds(),
          Cmds(p1Begin, p1Begin + groupSize),
          Cmds(p2Begin, commands.end()));
  }
  else {
    // If there are more than 12 commands, make the 12 last ones parallel
    auto sBegin  = commands.begin();
    auto p1Begin = sBegin  + cmdCount - 12;
    auto p2Begin = p1Begin + 6;
    return ParallelCommandSequence<Cmds>(
          Cmds(sBegin, p1Begin),
          Cmds(p1Begin, p2Begin),
          Cmds(p2Begin, commands.end()));
  }
}


template <typename Cmd, typename GenFunc>
class ParallelCommandsGen
{
public:
  using CmdSP = std::shared_ptr<const Cmd>;
  using Model = typename Cmd::Model;
  using Sut = typename Cmd::Sut;

  template <typename ModelArg, typename GenFuncArg>
  ParallelCommandsGen(ModelArg &&initialState, GenFuncArg &&genFunc)
    : commandsGen(std::forward<ModelArg>(initialState),
                  std::forward<GenFuncArg>(genFunc))
  {
  }

  Shrinkable<ParallelCommandSequence<Commands<Cmd>>> operator()(
    const Random &random,
    int size) const {
    return toParallelSequence(commandsGen(random, size));
  }
private:
  CommandsGen<Cmd, GenFunc> commandsGen;
};

} // detail

template <typename Cmd, typename GenerationFunc>
Gen<detail::ParallelCommandSequence<Commands<Cmd>>> parallelCommands(
  const typename Cmd::Model &initialState,
  GenerationFunc &&genFunc) {
  return detail::ParallelCommandsGen<Cmd, Decay<GenerationFunc>>(
      initialState, std::forward<GenerationFunc>(genFunc));
}

} // namespace gen
} // namespace state
} // namespace rc
