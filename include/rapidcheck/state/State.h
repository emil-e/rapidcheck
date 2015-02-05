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
    //! state. Default implementation simply returns the given state.
    //!
    //! Assert preconditions using `RC_PRE`. If preconditions do not hold,
    //! command will be discarded and a new one will be generated.
    virtual State nextState(const State &s0) const;

    //! Applies this command to the given system under test assuming it has the
    //! given state. Default implementation does nothing.
    //!
    //! Use rapidcheck assertion macros to check that the system behaves
    //! properly.
    virtual void run(const State &s0, Sut &sut) const;

    //! Outputs a human readable representation of the command to the given
    //! output stream.
    virtual void show(std::ostream &os) const;

    virtual ~Command() = default;
};

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

#include "State.hpp"
