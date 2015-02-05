#pragma once

namespace rc {
namespace gen {

template<typename Callable> class AnyInvocation;
template<typename Exception, typename Gen, typename Catcher> class Rescue;
template<typename Callable> class Lambda;

//! Wraps the given generator and catches any exception of type `Exception` and
//! translates them to ordinary return values using a catching function which
//! the exception as an argument and returns the translated value.
template<typename Exception, typename Gen, typename Catcher>
Rescue<Exception, Gen, Catcher> rescue(Gen generator, Catcher catcher);

//! Creates an anonymous generator that uses the given callable as the
//! generation method.
template<typename Callable>
Lambda<Callable> lambda(Callable callable);

//! Generates values by cwalling the given callable with randomly generated
//! arguments.
template<typename Callable>
AnyInvocation<Callable> anyInvocation(Callable callable);

} // namespace gen
} // namespace rc

#include "Invoke.hpp"
