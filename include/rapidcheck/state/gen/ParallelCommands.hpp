#pragma once

#include "rapidcheck/state/gen/Commands.h"

namespace rc {
namespace state {
namespace gen {
namespace detail {

template <typename Cmds>
struct ParallelCommandSequence {
  ParallelCommandSequence(const Shrinkable<Cmds> &prefix,
                          const Shrinkable<Cmds> &left,
                          const Shrinkable<Cmds> &right)
      : prefix(prefix)
      , left(left)
      , right(right) {}

  Shrinkable<Cmds> prefix;
  Shrinkable<Cmds> left;
  Shrinkable<Cmds> right;
};

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
    return seq::concat(
        shrinkPrefix(s), shrinkLeft(s), shrinkRight(s), unparallalize(s));
  }

  static Seq<ParallelCommandSequence<Commands<Cmd>>>
  unparallalize(const ParallelCommandSequence<Commands<Cmd>> &s) {
    auto s1prefix = s.prefix.value();
    auto s1left = s.left.value();
    auto leftHead = s1left.begin();
    s1prefix.push_back(*leftHead);
    s1left.erase(leftHead);

    auto s2prefix = s.prefix.value();
    auto s2right = s.right.value();
    auto rightHead = s2right.begin();
    s2prefix.push_back(*rightHead);
    s2right.erase(rightHead);

    return seq::just(
        ParallelCommandSequence<Commands<Cmd>>(
            shrinkable::just(s1prefix), shrinkable::just(s1left), s.right),
        ParallelCommandSequence<Commands<Cmd>>(
            shrinkable::just(s2prefix), s.left, shrinkable::just(s2right)));
  }

  static Seq<ParallelCommandSequence<Commands<Cmd>>>
  shrinkPrefix(const ParallelCommandSequence<Commands<Cmd>> &s) {
    auto shrunkSeqs = s.prefix.shrinks();
    return seq::map(std::move(shrunkSeqs),
                    [=](const Shrinkable<Commands<Cmd>> &commands) {
                      return ParallelCommandSequence<Commands<Cmd>>(
                          commands, s.left, s.right);
                    });
  }

  static Seq<ParallelCommandSequence<Commands<Cmd>>>
  shrinkLeft(const ParallelCommandSequence<Commands<Cmd>> &s) {
    auto shrunkSeqs = s.left.shrinks();
    return seq::map(std::move(shrunkSeqs),
                    [=](const Shrinkable<Commands<Cmd>> &commands) {
                      return ParallelCommandSequence<Commands<Cmd>>(
                          s.prefix, commands, s.right);
                    });
  }

  static Seq<ParallelCommandSequence<Commands<Cmd>>>
  shrinkRight(const ParallelCommandSequence<Commands<Cmd>> &s) {
    auto shrunkSeqs = s.right.shrinks();
    return seq::map(std::move(shrunkSeqs),
                    [=](const Shrinkable<Commands<Cmd>> &commands) {
                      return ParallelCommandSequence<Commands<Cmd>>(
                          s.prefix, s.left, commands);
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
  os << "Sequential sequence:" << std::endl;
  show(sequence.prefix.value(), os);
  os << "First parallel sequence:" << std::endl;
  show(sequence.left.value(), os);
  os << "Second parallel sequence:" << std::endl;
  show(sequence.right.value(), os);
}

} // detail
} // gen
} // state

template <typename Cmds>
struct ShowType<state::gen::detail::ParallelCommandSequence<Cmds>> {
  static void showType(std::ostream &os) {
    os << "Parallel command sequence";
    //detail::showType<Cmds>(os);
  }
};

namespace state {
namespace gen {

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
