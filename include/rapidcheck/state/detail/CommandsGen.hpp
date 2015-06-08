#pragma once

namespace rc {
namespace state {
namespace detail {

template <typename Cmd, typename GenFunc>
class CommandsGen {
public:
  using CmdSP = std::shared_ptr<const Cmd>;
  using State = typename Cmd::State;
  using Sut = typename Cmd::Sut;

  template <typename StateArg, typename GenFuncArg>
  CommandsGen(StateArg &&initialState, GenFuncArg &&genFunc)
      : m_initialState(std::forward<StateArg>(initialState))
      , m_genFunc(std::forward<GenFuncArg>(genFunc)) {}

  Shrinkable<Commands<Cmd>> operator()(const Random &random, int size) const {
    return generateCommands(random, size);
  }

private:
  struct CommandEntry {
    CommandEntry(Random &&aRandom,
                 Shrinkable<CmdSP> &&aShrinkable,
                 State &&aState)
        : random(std::move(aRandom))
        , shrinkable(std::move(aShrinkable))
        , postState(std::move(aState)) {}

    Random random;
    Shrinkable<CmdSP> shrinkable;
    State postState;
  };

  struct CommandSequence {
    CommandSequence(const State &initState, const GenFunc &func, int sz)
        : initialState(initState)
        , genFunc(func)
        , size(sz)
        , numFixed(0) {}

    State initialState;
    GenFunc genFunc;
    int size;
    std::size_t numFixed;
    std::vector<CommandEntry> entries;

    const State &stateAt(std::size_t i) const {
      if (i <= 0) {
        return initialState;
      }
      return entries[i - 1].postState;
    }

    void regenerateEntriesFrom(std::size_t start) {
      for (auto i = start; i < entries.size(); i++) {
        if (!regenerateEntryAt(i)) {
          entries.erase(begin(entries) + i--);
        }
      }
    }

    bool regenerateEntryAt(std::size_t i) {
      using namespace ::rc::detail;
      try {
        auto &entry = entries[i];
        const auto &preState = stateAt(i);
        entry.shrinkable = genFunc(preState)(entry.random, size);
        const auto cmd = entry.shrinkable.value();
        entry.postState = preState;
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
  };

  Shrinkable<Commands<Cmd>> generateCommands(const Random &random,
                                             int size) const {
    return shrinkable::map(generateSequence(random, size),
                           [](const CommandSequence &sequence) {
                             Commands<Cmd> cmds;
                             const auto &entries = sequence.entries;
                             cmds.commands.reserve(entries.size());
                             std::transform(begin(entries),
                                            end(entries),
                                            std::back_inserter(cmds.commands),
                                            [](const CommandEntry &entry) {
                                              return entry.shrinkable.value();
                                            });
                             return cmds;
                           });
  }

  Shrinkable<CommandSequence> generateSequence(const Random &random,
                                               int size) const {
    Random r(random);
    std::size_t count = (r.split().next() % (size + 1)) + 1;
    return shrinkable::shrinkRecur(generateInitial(random, size, count),
                                   &shrinkSequence);
  }

  CommandSequence
  generateInitial(const Random &random, int size, std::size_t count) const {
    CommandSequence sequence(m_initialState, m_genFunc, size);
    sequence.entries.reserve(count);

    auto *state = &m_initialState;
    auto r = random;
    while (sequence.entries.size() < count) {
      sequence.entries.push_back(nextEntry(r.split(), size, *state));
      state = &sequence.entries.back().postState;
    }

    return sequence;
  }

  CommandEntry
  nextEntry(const Random &random, int size, const State &state) const {
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

  static Seq<CommandSequence> shrinkSequence(const CommandSequence &s) {
    return seq::concat(shrinkRemoving(s), shrinkIndividual(s));
  }

  static Seq<CommandSequence> shrinkRemoving(const CommandSequence &s) {
    auto nonEmptyRanges = seq::subranges(s.numFixed, s.entries.size());
    return seq::map(std::move(nonEmptyRanges),
                    [=](const std::pair<std::size_t, std::size_t> &r) {
                      auto shrunk = s;
                      shrunk.entries.erase(begin(shrunk.entries) + r.first,
                                           begin(shrunk.entries) + r.second);
                      shrunk.regenerateEntriesFrom(r.first);
                      return shrunk;
                    });
  }

  static Seq<CommandSequence> shrinkIndividual(const CommandSequence &s) {
    return seq::mapcat(seq::range(s.numFixed, s.entries.size()),
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
                                           entry.postState = shrunk.stateAt(i);
                                           entry.shrinkable.value()->apply(
                                               entry.postState);
                                           shrunk.regenerateEntriesFrom(i + 1);

                                           shrunk.numFixed = i;
                                           return shrunk;
                                         });
                       });
  }

  State m_initialState;
  GenFunc m_genFunc;
};

template <typename Cmd, typename State, typename GenerationFunc>
Gen<Commands<Cmd>> genCommands(State &&initialState, GenerationFunc &&genFunc) {
  return detail::CommandsGen<Cmd, Decay<GenerationFunc>>(
      std::forward<State>(initialState), std::forward<GenerationFunc>(genFunc));
}

} // namespace detail
} // namespace state
} // namespace rc
