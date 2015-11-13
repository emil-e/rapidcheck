#pragma once

#include <memory>

#include "rapidcheck/Gen.h"

namespace rc {
namespace state {

/// Tests a stateful system. This function has assertion semantics (i.e. a
/// failure is equivalent to calling `RC_FAIL` and success is equivalent to
/// `RC_SUCCEED`) so it is intended to be used from a property.
///
/// @param initialState    The initial model state.
/// @param sut             The system under test.
/// @param generationFunc  A callable which takes the current model state as a
///                        parameter and returns a generator for a (possibly)
///                        suitable command.
template <typename Model, typename Sut, typename GenFunc>
void check(const Model &initialState, Sut &sut, GenFunc &&generationFunc);

/// Tests a stateful system for race conditions by executing some commands
/// in parallel (i.e. concurrently in different threads). This function has
/// assertion semantics (i.e. a failure is equivalent to calling `RC_FAIL` and
/// success is equivalent to `RC_SUCCEED`) so it is intended to be used from a
/// property. Note that all comands have to implement the overload of the run
/// function that has the following signature 'std::function<void(const
/// ModelT&)> run(Sut &sut) const'
///
/// @param initialState    The initial model state.
/// @param sut             The system under test.
/// @param generationFunc  A callable which takes the current model state as a
///                        parameter and returns a generator for a (possibly)
///                        suitable command.
template <typename Model, typename Sut, typename GenFunc>
void checkParallel(const Model &initialState, Sut &sut, GenFunc &&generationFunc);

/// Checks whether command is valid for the given state.
template <typename Model, typename Sut>
bool isValidCommand(const Command<Model, Sut> &command, const Model &s0);

} // namespace state
} // namespace rc

#include "State.hpp"
