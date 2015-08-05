#pragma once

#include "rapidcheck/state/gen/Commands.h"

namespace rc {
namespace state {
namespace gen {
namespace detail {

template <typename Cmds>
struct ParallelCommandSequence {
  ParallelCommandSequence(const Shrinkable<Cmds> &serialCmdSeq,
                          const Shrinkable<Cmds> &parallelCmdSeq1,
                          const Shrinkable<Cmds> &parallelCmdSeq2)
      : serialCmdSeq(serialCmdSeq)
      , parallelCmdSeq1(parallelCmdSeq1)
      , parallelCmdSeq2(parallelCmdSeq2) {}

  Shrinkable<Cmds> serialCmdSeq;
  Shrinkable<Cmds> parallelCmdSeq1;
  Shrinkable<Cmds> parallelCmdSeq2;
};

template <typename Cmds>
ParallelCommandSequence<Cmds> toParallelSequence(const Cmds &commands) {
  // TODO: Refactor to a single case.
  auto cmdCount = commands.size();
  // If there are fewer than 12 commands, make all parallel
  if (cmdCount < 12) {
    auto groupSize = cmdCount / 2;
    auto p1Begin = commands.begin();
    auto p2Begin = p1Begin + groupSize;
    return ParallelCommandSequence<Cmds>(Cmds(),
                                         Cmds(p1Begin, p1Begin + groupSize),
                                         Cmds(p2Begin, commands.end()));
  } else {
    // If there are more than 12 commands, make the 12 last ones parallel
    auto sBegin = commands.begin();
    auto p1Begin = sBegin + cmdCount - 12;
    auto p2Begin = p1Begin + 6;
    return ParallelCommandSequence<Cmds>(Cmds(sBegin, p1Begin),
                                         Cmds(p1Begin, p2Begin),
                                         Cmds(p2Begin, commands.end()));
  }
}

template <typename Cmd, typename GenFunc>
class ParallelCommandsGen {
public:
  using CmdSP = std::shared_ptr<const Cmd>;
  using Model = typename Cmd::Model;
  using Sut = typename Cmd::Sut;

  template <typename ModelArg, typename GenFuncArg>
  ParallelCommandsGen(ModelArg &&initialState, GenFuncArg &&genFunc)
      : commandsGen(std::forward<ModelArg>(initialState),
                    std::forward<GenFuncArg>(genFunc)) {}

  Shrinkable<ParallelCommandSequence<Commands<Cmd>>>
  operator()(const Random &random, int size) const {
    return generateCommands(random, size);
  }

private:
  Shrinkable<ParallelCommandSequence<Commands<Cmd>>>
  generateCommands(const Random &random, int size) const {
    Random r(random);
    std::size_t count = (r.split().next() % (size + 1)) + 1;
    return shrinkable::shrinkRecur(generateInitial(random, size),
                                   &shrinkSequence);
  }

  ParallelCommandSequence<Commands<Cmd>> generateInitial(const Random &random,
                                                         int size) const {
    auto commandDistribution = parallelCommandDistribution(size);
    auto r = random;
    return ParallelCommandSequence<Commands<Cmd>>(
        commandsGen(r.split().next(), std::get<0>(commandDistribution)),
        commandsGen(r.split().next(), std::get<1>(commandDistribution)),
        commandsGen(r.split().next(), std::get<2>(commandDistribution)));
  }

  static Seq<ParallelCommandSequence<Commands<Cmd>>>
  shrinkSequence(const ParallelCommandSequence<Commands<Cmd>> &s) {
    return seq::concat(shrinkPrefix(s));
  }

  static Seq<ParallelCommandSequence<Commands<Cmd>>>
  shrinkPrefix(const ParallelCommandSequence<Commands<Cmd>> &s) {
    auto shrunkPrefixSeq = s.serialCmdSeq.shrinks();
    return seq::map(std::move(shrunkPrefixSeq),
                    [=](const Shrinkable<Commands<Cmd>> &prefix) {
                      return ParallelCommandSequence<Commands<Cmd>>(
                          prefix, s.parallelCmdSeq1, s.parallelCmdSeq2);
                    });
  }

  /// Calculates how many commands to generate for each subsequence.
  /// Returns a three tuple with the number of commands for {prefix,
  /// first parallel sequence, second parallel sequence}
  static std::tuple<int, int, int> parallelCommandDistribution(int cmdCount) {
    if (cmdCount < 12) {
      // If there are fewer than 12 commands, make all parallel
      return std::make_tuple(0, cmdCount / 2, cmdCount - cmdCount / 2);
    } else {
      // If there are more than 12 commands, make the 12 last ones parallel
      return std::make_tuple(cmdCount - 12, 6, 6);
    }
  }

  CommandsGen<Cmd, GenFunc> commandsGen;
};

template <typename Cmds>
void showValue(const ParallelCommandSequence<Cmds> &sequence,
               std::ostream &os) {
  os << "Sequential:" << std::endl;
  show(sequence.serialCmdSeq.value(), os);
  os << "Parallel:" << std::endl;
  show(sequence.parallelCmdSeq1.value(), os);
  os << "," << std::endl;
  show(sequence.parallelCmdSeq2.value(), os);
}

} // detail

template <typename Cmd, typename GenerationFunc>
Gen<detail::ParallelCommandSequence<Commands<Cmd>>>
parallelCommands(const typename Cmd::Model &initialState,
                 GenerationFunc &&genFunc) {
  return detail::ParallelCommandsGen<Cmd, Decay<GenerationFunc>>(
      initialState, std::forward<GenerationFunc>(genFunc));
}

} // namespace gen
} // namespace state
} // namespace rc
