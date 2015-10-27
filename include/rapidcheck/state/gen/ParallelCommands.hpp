#pragma once

#include "rapidcheck/Random.h"
#include "rapidcheck/GenerationFailure.h"
#include "rapidcheck/shrinkable/Transform.h"
#include "rapidcheck/state/gen/CommandSequence.hpp"

namespace rc {
namespace state {
namespace gen {
namespace detail {

template <typename Cmd, typename GenFunc>
class ParallelCommandsGen {
public:
  using CmdSP = std::shared_ptr<const Cmd>;
  using Model = typename Cmd::Model;
  using Sut = typename Cmd::Sut;

  template <typename ModelArg, typename GenFuncArg>
  ParallelCommandsGen(ModelArg &&initialState, GenFuncArg &&genFunc)
      : m_initialState(std::forward<ModelArg>(initialState))
      , m_genFunc(std::forward<GenFuncArg>(genFunc)) {}

  Shrinkable<ParallelCommands<Cmd>> operator()(const Random &random,
                                               int size) const {
    return generateCommands(random, size);
  }

private:
  using CommandSequence = CommandSequence<Cmd, GenFunc>;
  using CommandEntry = CommandEntry<Cmd, GenFunc>;

  struct ParallelCommandSequence {
    ParallelCommandSequence(CommandSequence prefix,
                            CommandSequence left,
                            CommandSequence right)
        : prefix(prefix)
        , left(left)
        , right(right) {}

    CommandSequence prefix;
    CommandSequence left;
    CommandSequence right;
  };

  Shrinkable<ParallelCommands<Cmd>> generateCommands(const Random &random,
                                                     int size) const {
    return shrinkable::map(generateParallelSequence(random, size),
                           [](const ParallelCommandSequence &sequence) {
                             ParallelCommands<Cmd> cmds;
                             copyCommandEntries(sequence.prefix, cmds.prefix);
                             copyCommandEntries(sequence.left, cmds.left);
                             copyCommandEntries(sequence.right, cmds.right);
                             return cmds;
                           });
  }

  template <typename T>
  static void copyCommandEntries(const CommandSequence &source,
                                 std::vector<T> &dest) {
    const auto &entries = source.entries;
    dest.reserve(entries.size());
    std::transform(
        begin(entries),
        end(entries),
        std::back_inserter(dest),
        [](const CommandEntry &entry) { return entry.shrinkable.value(); });
  }

  Shrinkable<ParallelCommandSequence>
  generateParallelSequence(const Random &random, int size) const {
    return shrinkable::shrinkRecur(generateInitialParallel(random, size),
                                   &shrinkSequence);
  }

  ParallelCommandSequence generateInitialParallel(const Random &random,
                                                  int size) const {
    auto r = random;
    int prefixCount, leftCount, rightCount;
    std::tie(prefixCount, leftCount, rightCount) =
        parallelCommandDistribution(r.split(), size);

    auto prefix = generateInitial(m_initialState, r.split(), size, prefixCount);

    return ParallelCommandSequence(
        prefix,
        generateInitial(prefix.postState(), r.split(), size, leftCount),
        generateInitial(prefix.postState(), r.split(), size, rightCount));
  }

  CommandSequence generateInitial(const Model &initialState,
                                  const Random &random,
                                  int size,
                                  std::size_t count) const {
    CommandSequence sequence(initialState, m_genFunc, size);
    sequence.entries.reserve(count);

    auto *state = &initialState;
    auto r = random;
    while (sequence.entries.size() < count) {
      sequence.entries.push_back(nextEntry(r.split(), size, *state));
      state = &sequence.entries.back().postState;
    }

    return sequence;
  }

  CommandEntry
  nextEntry(const Random &random, int size, const Model &state) const {
    using namespace ::rc::detail;
    auto r = random;
    const auto gen = m_genFunc(state);
    // TODO configurability?
    for (int tries = 0; tries < 100; tries++) {
      try {
        auto random = r.split();
        auto shrinkable = gen(random, size);
        auto postState = state;
        shrinkable.value()->apply(postState);

        return CommandEntry(
            std::move(random), std::move(shrinkable), std::move(postState));
      } catch (const CaseResult &result) {
        if (result.type != CaseResult::Type::Discard) {
          throw;
        }
        // What to do?
      } catch (const GenerationFailure &failure) {
        // What to do?
      }
    }

    // TODO better error message
    throw GenerationFailure("Failed to generate command after 100 tries.");
  }

  static Seq<ParallelCommandSequence>
  shrinkSequence(const ParallelCommandSequence &s) {
    return seq::concat(shrinkPrefix(s),
                       shrinkLeft(s),
                       shrinkRight(s),
                       unparallelizeLeft(s),
                       unparallelizeRight(s));
  }

  static Seq<ParallelCommandSequence>
  unparallelizeLeft(const ParallelCommandSequence &s) {
    auto elemCounts = seq::range(static_cast<int>(s.left.entries.size()), 0);

    return seq::map(std::move(elemCounts),
                    [s](const std::size_t &elemCount) {
                      auto prefix = s.prefix;
                      auto left = s.left;
                      moveElements(left.entries, prefix.entries, elemCount);

                      return ParallelCommandSequence(prefix, left, s.right);
                    });
  }

  static Seq<ParallelCommandSequence>
  unparallelizeRight(const ParallelCommandSequence &s) {
    auto elemCounts = seq::range(static_cast<int>(s.right.entries.size()), 0);

    return seq::map(std::move(elemCounts),
                    [s](const std::size_t &elemCount) {
                      auto prefix = s.prefix;
                      auto right = s.right;
                      moveElements(right.entries, prefix.entries, elemCount);

                      return ParallelCommandSequence(prefix, s.left, right);
                    });
  }

  /// Move `count` elements from begin of `source` to end of `dest` and
  /// erases the moved elements from source.
  static void moveElements(std::vector<CommandEntry> &source,
                           std::vector<CommandEntry> &dest,
                           int count) {
    auto sourceEndIt = std::next(begin(source), count);
    std::move(begin(source), sourceEndIt, std::back_inserter(dest));
    source.erase(begin(source), sourceEndIt);
  }

  static Seq<ParallelCommandSequence>
  shrinkPrefix(const ParallelCommandSequence &s) {
    auto individualShrinks =
        seq::map(shrinkIndividual(s.prefix),
                 [=](const CommandSequence &commands) {
                   return ParallelCommandSequence(commands, s.left, s.right);
                 });
    return seq::concat(shrinkRemovingPrefix(s), individualShrinks);
  }

  static Seq<ParallelCommandSequence>
  shrinkLeft(const ParallelCommandSequence &s) {
    auto shrunkSeqs =
        seq::concat(shrinkRemoving(s.left), shrinkIndividual(s.left));
    return seq::map(std::move(shrunkSeqs),
                    [=](const CommandSequence &commands) {
                      return ParallelCommandSequence(
                          s.prefix, commands, s.right);
                    });
  }

  static Seq<ParallelCommandSequence>
  shrinkRight(const ParallelCommandSequence &s) {
    auto shrunkSeqs =
        seq::concat(shrinkRemoving(s.right), shrinkIndividual(s.right));
    return seq::map(std::move(shrunkSeqs),
                    [=](const CommandSequence &commands) {
                      return ParallelCommandSequence(
                          s.prefix, s.left, commands);
                    });
  }

  static Seq<CommandSequence> shrinkRemoving(const CommandSequence &s) {
    auto nonEmptyRanges = seq::subranges(0, s.entries.size());
    return seq::map(std::move(nonEmptyRanges),
                    [=](const std::pair<std::size_t, std::size_t> &r) {
                      auto shrunk = s;
                      shrunk.entries.erase(begin(shrunk.entries) + r.first,
                                           begin(shrunk.entries) + r.second);
                      shrunk.repairEntriesFrom(r.first);
                      return shrunk;
                    });
  }

  static Seq<ParallelCommandSequence>
  shrinkRemovingPrefix(const ParallelCommandSequence &s) {
    auto nonEmptyRanges = seq::subranges(0, s.prefix.entries.size());
    return seq::map(std::move(nonEmptyRanges),
                    [=](const std::pair<std::size_t, std::size_t> &r) {
                      auto shrunk = s;
                      auto &prefix = shrunk.prefix;

                      // Remove elements from prefix
                      prefix.entries.erase(begin(prefix.entries) + r.first,
                                           begin(prefix.entries) + r.second);
                      prefix.repairEntriesFrom(r.first);

                      // Regenerate left
                      shrunk.left.initialState = prefix.postState();
                      shrunk.left.repairEntriesFrom(0);

                      // Regenerate right
                      shrunk.right.initialState = prefix.postState();
                      shrunk.right.repairEntriesFrom(0);

                      return shrunk;
                    });
  }

  static Seq<CommandSequence> shrinkIndividual(const CommandSequence &s) {
    return seq::mapcat(seq::range<std::size_t>(0, s.entries.size()),
                       [=](std::size_t i) {
                         const auto &preState = s.stateAt(i);
                         auto valid = seq::filter(
                             s.entries[i].shrinkable.shrinks(),
                             [=](const Shrinkable<CmdSP> &s) {
                               return isValidCommand(*s.value(), preState);
                             });

                         return seq::map(std::move(valid),
                                         [=](Shrinkable<CmdSP> &&cmd) {
                                           auto shrunk = s;
                                           auto &entry = shrunk.entries[i];

                                           entry.shrinkable = std::move(cmd);
                                           shrunk.repairEntriesFrom(i);

                                           return shrunk;
                                         });
                       });
  }

  /// Calculates the maximum number of commands to generate for each
  /// subsequence. Returns a three tuple with the number of commands for
  /// {prefix, left, right}
  static std::tuple<int, int, int>
  parallelCommandDistribution(const Random &random, int count) {
    auto r = random;
    if (count <= 12) {
      // Put all commands in the parallel branches
      std::size_t left = (r.next() % (count / 2 + 1));
      std::size_t right = (r.next() % (count / 2 + 1));
      return std::make_tuple(0, left, right);
    } else {
      // Put at most 12 commands in the parallel branches
      std::size_t prefix = r.next() % (count - 12);
      std::size_t left = r.next() % 6;
      std::size_t right = r.next() % 6;
      return std::make_tuple(prefix, left, right);
    }
  }

  Model m_initialState;
  GenFunc m_genFunc;
};

} // detail

template <typename Cmd, typename GenerationFunc>
Gen<ParallelCommands<Cmd>>
parallelCommands(const typename Cmd::Model &initialState,
                 GenerationFunc &&genFunc) {
  return detail::ParallelCommandsGen<Cmd, Decay<GenerationFunc>>(
      initialState, std::forward<GenerationFunc>(genFunc));
}

} // namespace gen
} // namespace state

template <typename Cmd>
struct ShowType<state::ParallelCommands<Cmd>> {
  static void showType(std::ostream &os) {
    os << "Parallel command sequence of ";
    detail::showType<Cmd>(os);
  }
};

} // namespace rc
