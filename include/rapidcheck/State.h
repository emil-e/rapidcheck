#pragma once

#include <memory>

namespace rc {
namespace state {

//! Base class for commands used in testing of stateful systems.
template<typename State, typename Sut>
class Command : public std::enable_shared_from_this<Command<State, Sut>>
{
public:
    typedef State StateT;
    typedef Sut SutT;

    //! Returns the state resulting from applying this command to the given
    //! state.
    virtual State nextState(const State &state) const = 0;

    //! Applies this command to the given system under test assuming it has the
    //! given state.
    virtual void run(const State &state, Sut &sut) const = 0;

    //! Outputs a human readable representation of the command to the given
    //! output stream.
    virtual void show(std::ostream &os) const;

    virtual ~Command() = default;
};

template<typename State, typename Sut>
void show(const Command<State, Sut> &command, std::ostream &os);

template<typename State, typename Sut>
using CommandSP = std::shared_ptr<Command<State, Sut>>;

//! Tests a stateful system. This function has assertion semantics (i.e. a
//! failure is equivalent to calling `RC_FAIL` and success is equivalent to
//! `RC_SUCCEED`) so it is intended to be used from a property.
//!
//! @param initialState    The initial model state.
//! @param sut             The system under test.
//! @param generationFunc  A callable which takes the current model state as a
//!                        parameter and generates a (possibly) suitable command.
template<typename State, typename Sut, typename GenerationFunc>
void check(State initialState,
           Sut &sut,
           GenerationFunc generationFunc);

} // namespace state
} // namespace rc

#include "detail/State.hpp"
