#pragma once

#include "rapidcheck/state/Commands.h"

namespace rc {
namespace state {
namespace gen {

template <typename Cmd, typename GenerationFunc>
Gen<Commands<Cmd>> commandsScaledLength(const typename Cmd::Model &initialState,
                                        double scale,
                                        GenerationFunc &&genFunc) {

  /// Generate a sequence of commands where the commands themself
  /// get passed the size ``size``.
  auto commands_with_size = [=](int size) {
    return commands<Cmd>(initialState, [=](const typename Cmd::Model &state) {
      return rc::gen::resize(size, genFunc(state));
    });
  };

  return rc::gen::withSize([=](int size) {
    return rc::gen::scale(scale, commands_with_size(size));
  });
}

} // namespace gen
} // namespace state
} // namespace rc
