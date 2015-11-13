#pragma once

#include "rapidcheck/Random.h"
#include "rapidcheck/Shrinkable.h"

namespace rc {
namespace state {
namespace gen {
namespace detail {

template <typename Cmd, typename GenFunc>
struct CommandEntry {
  using CmdSP = std::shared_ptr<const Cmd>;
  using Model = typename Cmd::Model;

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

template <typename Cmd, typename GenFunc>
struct CommandSequence {
  using Model = typename Cmd::Model;

  CommandSequence(const Model &initState, const GenFunc &func, int sz)
      : initialState(initState)
      , genFunc(func)
      , size(sz) {}

  Model initialState;
  GenFunc genFunc;
  int size;
  std::vector<CommandEntry<Cmd, GenFunc>> entries;

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
      } catch (const GenerationFailure &) {
        // Just return false below
      }

      return false;
    }

    const Model& postState() {
      return stateAt(entries.size());
    }
  };


} // namespace detail
} // namespace gen
} // namespace state
} // namespace rc
