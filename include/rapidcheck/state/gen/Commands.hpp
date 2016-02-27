#pragma once

#include <algorithm>

#include "rapidcheck/Random.h"
#include "rapidcheck/GenerationFailure.h"
#include "rapidcheck/shrinkable/Transform.h"

namespace rc {
namespace state {
namespace gen {
namespace detail {

template <typename Cmd, typename GenFunc>
class CommandsGen {
public:
  using CmdSP = std::shared_ptr<const Cmd>;
  using Model = typename Cmd::Model;
  using Sut = typename Cmd::Sut;

  template <typename ModelArg, typename GenFuncArg>
  CommandsGen(ModelArg &&initialState, GenFuncArg &&genFunc)
      : m_initialState(std::forward<ModelArg>(initialState))
      , m_genFunc(std::forward<GenFuncArg>(genFunc)) {}

  Shrinkable<Commands<Cmd>> operator()(const Random &random,
                                          int size) const {
    return generateCommands(random, size);
  }

private:
  struct CommandEntry {
    CommandEntry(Random &&aRandom, Shrinkable<CmdSP> &&aShrinkable)
        : random(std::move(aRandom))
        , shrinkable(std::move(aShrinkable)) {}

    Random random;
    Shrinkable<CmdSP> shrinkable;
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

    Model stateAt(std::size_t n) const {
      auto state = initialState;
      for (std::size_t i = 0; i < n; i++) {
        entries[i].shrinkable.value()->apply(state);
      }

      return state;
    }

    void repairEntries() {
      auto state = initialState;
      for (std::size_t i = 0; i < entries.size(); i++) {
        if (!repairEntryAt(i, state)) {
          entries.erase(begin(entries) + i--);
        }
      }
    }

    bool repairEntryAt(std::size_t i, Model &state) {
      auto &entry = entries[i];
      const auto cmd = entry.shrinkable.value();
      if (!isValidCommand(*cmd, state)) {
        return regenerateEntryAt(i, state);
      }
      cmd->apply(state);
      return true;
    }

    bool regenerateEntryAt(std::size_t i, Model &state) {
      try {
        auto &entry = entries[i];
        entry.shrinkable = genFunc(state)(entry.random, size);
        const auto cmd = entry.shrinkable.value();
        if (!isValidCommand(*cmd, state)) {
          return false;
        }
        cmd->apply(state);
      } catch (const GenerationFailure &) {
        return false;
      }

      return true;
    }
  };

  Shrinkable<Commands<Cmd>> generateCommands(const Random &random,
                                                int size) const {
    return shrinkable::map(generateSequence(random, size),
                           [](const CommandSequence &sequence) {
                             Commands<Cmd> cmds;
                             const auto &entries = sequence.entries;
                             cmds.reserve(entries.size());
                             std::transform(begin(entries),
                                            end(entries),
                                            std::back_inserter(cmds),
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

    auto state = m_initialState;
    auto r = random;
    while (sequence.entries.size() < count) {
      sequence.entries.push_back(nextEntry(r.split(), size, state));
    }

    return sequence;
  }

  CommandEntry nextEntry(const Random &random, int size, Model &state) const {
    auto r = random;
    const auto gen = m_genFunc(state);
    // TODO configurability?
    for (int tries = 0; tries < 100; tries++) {
      try {
        auto random = r.split();
        auto shrinkable = gen(random, size);
        auto cmd = shrinkable.value();
        if (!isValidCommand(*cmd, state)) {
          continue;
        }
        cmd->apply(state);

        return CommandEntry(std::move(random), std::move(shrinkable));
      } catch (const GenerationFailure &) {
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
    auto nonEmptyRanges = seq::subranges(0, s.entries.size());
    return seq::map(std::move(nonEmptyRanges),
                    [=](const std::pair<std::size_t, std::size_t> &r) {
                      auto shrunk = s;
                      shrunk.entries.erase(begin(shrunk.entries) + r.first,
                                           begin(shrunk.entries) + r.second);
                      shrunk.repairEntries();
                      return shrunk;
                    });
  }

  static Seq<CommandSequence> shrinkIndividual(const CommandSequence &s) {
    return seq::mapcat(seq::range<std::size_t>(0, s.entries.size()),
                       [=](std::size_t i) {
                         const auto preState = s.stateAt(i);
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
                                           shrunk.repairEntries();

                                           return shrunk;
                                         });
                       });
  }

  Model m_initialState;
  GenFunc m_genFunc;
};

} // namespace detail

template <typename Cmd, typename GenerationFunc>
Gen<Commands<Cmd>> commands(const typename Cmd::Model &initialState,
                            GenerationFunc &&genFunc) {
  return detail::CommandsGen<Cmd, Decay<GenerationFunc>>(
      initialState, std::forward<GenerationFunc>(genFunc));
}

} // namespace gen
} // namespace state
} // namespace rc
