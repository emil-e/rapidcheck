#pragma once

#include "Suite.h"

namespace rc {
namespace detail {

namespace param {

//! The current `TestGroup`.
struct CurrentGroup { typedef TestGroup *ValueType; };

} // namespace param

//! Helper class for automatically running a function on initialization.
class StaticInitializer
{
public:
    //! Runs the given function with the given arguments when instantiated.
    //!
    //! @param func    The function to run.
    //! @param args... The arguments to pass.
    template<typename Callable, typename ...Args>
    StaticInitializer(Callable callable, Args ...args)
    { callable(std::forward<Args>(args)...); }
};

} // namespace detail

template<typename Constructor>
void describe(std::string description, Constructor constructor)
{
    using namespace detail;
    TestGroup group(std::move(description));
    ImplicitParam<param::CurrentGroup> currentGroup;
    currentGroup.let(&group);
    constructor();
    TestSuite::defaultSuite().add(std::move(group));
}

template<typename Testable>
void it(std::string description, Testable testable)
{
    using namespace detail;
    ImplicitParam<param::CurrentGroup> currentGroup;
    assert(currentGroup.hasBinding());
    auto generator = gen::anyInvocation(testable);
    static_assert(
        std::is_same<bool, decltype(generator())>::value,
        "A property must return bool");
    (*currentGroup)->add(Property(std::move(description),
                                  std::move(generator),
                                  PropertyParams()));
}

} // namespace rc
