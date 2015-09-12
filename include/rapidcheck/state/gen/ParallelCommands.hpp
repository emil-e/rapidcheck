#pragma once

#include "rapidcheck/state/gen/Commands.h"

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
  struct CommandEntry {
    CommandEntry(Random &&aRandom,
                 Shrinkable<CmdSP> &&aShrinkable,
                 Model &&aState)
        : random(std::move(aRandom))
        , shrinkable(std::move(aShrinkable))
        , postState(std::move(aState)) {}

    Random random;
    Shrinkable<CmdSP> shrinkable;
    Model postState;
  };

  struct CommandSequence {
    CommandSequence(const Model &initState, const GenFunc &func, int sz)
        : initialState(initState)
        , genFunc(func)
        , size(sz) {}

    Model initialState;
    GenFunc genFunc;
    int size;
    std::vector<CommandEntry> entries;

    const Model &stateAt(std::size_t i) const {
      if (i <= 0) {
        return initialState;
      }
      return entries[i - 1].postState;
    }

    void repairEntriesFrom(std::size_t start) {
      for (auto i = start; i < entries.size(); i++) {
        if (!repairEntryAt(i)) {
          entries.erase(begin(entries) + i--);
        }
      }
    }

    bool repairEntryAt(std::size_t i) {
      using namespace ::rc::detail;
      try {
        auto &entry = entries[i];
        const auto cmd = entry.shrinkable.value();
        entry.postState = stateAt(i);
        cmd->apply(entry.postState);
      } catch (const CaseResult &result) {
        if (result.type != CaseResult::Type::Discard) {
          throw;
        }

        return regenerateEntryAt(i);
      }

      return true;
    }

    bool regenerateEntryAt(std::size_t i) {
      using namespace ::rc::detail;
      try {
        auto &entry = entries[i];
        const auto &preState = stateAt(i);
        entry.shrinkable = genFunc(preState)(entry.random, size);
        const auto cmd = entry.shrinkable.value();
        entry.postState = preState;
        // NOTE: Apply might throw which leaves us with an incorrect postState
        // but that's okay since the entry will discarded anyway in that case.
        cmd->apply(entry.postState);
        return true;
      } catch (const CaseResult &result) {
        if (result.type != CaseResult::Type::Discard) {
          throw;
        }
      } catch (const GenerationFailure &failure) {
        // Just return false below
      }

      return false;
    }

    const Model& postState() {
      return stateAt(entries.size());
    }
  };

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
    auto r1 = random;
    auto r2 = r1.split();
    int prefixSz, leftSz, rightSz;
    std::tie(prefixSz, leftSz, rightSz) = parallelCommandDistribution(size);

    std::size_t prefixCount = (r1.next() % (prefixSz + 1));
    std::size_t leftCount = (r1.next() % (leftSz + 1));
    std::size_t rightCount = (r1.next() % (rightSz + 1));

    auto prefix =
      generateInitial(m_initialState, r2.split(), prefixSz, prefixCount);

    return ParallelCommandSequence(
        prefix,
        generateInitial(prefix.postState(), r2.split(), leftSz, leftCount),
        generateInitial(prefix.postState(), r2.split(), rightSz, rightCount));
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
                       unparallelize(s));
  }

  static Seq<ParallelCommandSequence>
  unparallelize(const ParallelCommandSequence &s) {
    auto elemsToMove = seq::filter(
        seq::combinations(
            seq::range(static_cast<int>(s.left.entries.size()), -1),
            seq::range(static_cast<int>(s.right.entries.size()), -1)),
        [](const std::pair<int, int> &elemCounts) {
          return elemCounts != std::make_pair(0, 0);
        });

    return seq::map(
        std::move(elemsToMove),
        [s](const std::pair<int, int> &elemCounts) {
          auto prefix = s.prefix;

          // move elements from left to prefix
          auto left = s.left;
          moveElements(left.entries, prefix.entries, std::get<0>(elemCounts));

          // move elements from right to prefix
          auto right = s.right;
          moveElements(right.entries, prefix.entries, std::get<1>(elemCounts));

          return ParallelCommandSequence(prefix, left, right);
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
                         return seq::map(s.entries[i].shrinkable.shrinks(),
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
  static std::tuple<int, int, int> parallelCommandDistribution(int cmdCount) {
    if (cmdCount < 12) {
      // If there are fewer than 12 commands, make all parallel
      return std::make_tuple(0, cmdCount / 2, cmdCount - cmdCount / 2);
    } else {
      // If there are more than 12 commands, make the 12 last ones parallel
      return std::make_tuple(cmdCount - 12, 6, 6);
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
    os << "Parallel command sequence ";
    detail::showType<Cmd>(os);
  }
};

} // namespace rc
