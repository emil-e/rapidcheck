#pragma once

#include <algorithm>

#include "rapidcheck/Random.h"
#include "rapidcheck/GenerationFailure.h"
#include "rapidcheck/shrinkable/Transform.h"
#include "rapidcheck/state/gen/CommandSequence.hpp"

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

  Shrinkable<Commands<Cmd>> operator()(const Random &random, int size) const {
    return generateCommands(random, size);
  }

private:
  using CmdSequence = CommandSequence<Cmd, GenFunc>;
  using CmdEntry = CommandEntry<Cmd, GenFunc>;

  Shrinkable<Commands<Cmd>> generateCommands(const Random &random,
                                             int size) const {
    return shrinkable::map(generateSequence(random, size),
                           [](const CmdSequence &sequence) {
                             Commands<Cmd> cmds;
                             const auto &entries = sequence.entries;
                             cmds.reserve(entries.size());
                             std::transform(begin(entries),
                                            end(entries),
                                            std::back_inserter(cmds),
                                            [](const CmdEntry &entry) {
                                              return entry.shrinkable.value();
                                            });
                             return cmds;
                           });
  }

  Shrinkable<CmdSequence> generateSequence(const Random &random,
                                           int size) const {
    Random r(random);
    std::size_t count = (r.split().next() % (size + 1)) + 1;
    return shrinkable::shrinkRecur(generateInitial(random, size, count),
                                   &shrinkSequence);
  }

  CmdSequence
  generateInitial(const Random &random, int size, std::size_t count) const {
    CmdSequence sequence(m_initialState, m_genFunc, size);
    sequence.entries.reserve(count);

    auto *state = &m_initialState;
    auto r = random;
    while (sequence.entries.size() < count) {
      sequence.entries.push_back(nextEntry(r.split(), size, *state));
      state = &sequence.entries.back().postState;
    }

    return sequence;
  }

  CmdEntry nextEntry(const Random &random, int size, const Model &state) const {
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

        return CmdEntry(
            std::move(random), std::move(shrinkable), std::move(postState));
      } catch (const CaseResult &result) {
        if (result.type != CaseResult::Type::Discard) {
          throw;
        }
        // What to do?
      } catch (const GenerationFailure &) {
        // What to do?
      }
    }

    // TODO better error message
    throw GenerationFailure("Failed to generate command after 100 tries.");
  }

  static Seq<CmdSequence> shrinkSequence(const CmdSequence &s) {
    return seq::concat(shrinkRemoving(s), shrinkIndividual(s));
  }

  static Seq<CmdSequence> shrinkRemoving(const CmdSequence &s) {
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

  static Seq<CmdSequence> shrinkIndividual(const CmdSequence &s) {
    return seq::mapcat(
      seq::range<std::size_t>(0, s.entries.size()),
        [=](std::size_t i) {
          const auto &preState = s.stateAt(i);
          auto valid =
              seq::filter(s.entries[i].shrinkable.shrinks(),
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
