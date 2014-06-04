#pragma once

#include "Generator.h"

namespace rc {

//! Namespace-level version of `describe`.
#define DESCRIBE(desc)                                                  \
    static void RC_UNIQUE(propGroup)();                                 \
    static detail::StaticInitializer RC_UNIQUE(propGroupInit)(          \
        ::rc::describe<void (*)()>,                                     \
        (desc),                                                         \
        RC_UNIQUE(propGroup));                                          \
    static void RC_UNIQUE(propGroup)()

//! Adds a property group.
//!
//! @param description  A description of the property group.
//! @param constructor  A callable which adds the properties of the property
//!                     group. Most likely a lambda.
template<typename Constructor>
void describe(std::string description, Constructor constructor);

//! Adds a property to the current property group.
//!
//! @param description  A description of the property.
//! @param testable     A callable which implements the property.
template<typename Testable>
void it(std::string description, Testable testable);

//! Runs the all registered properties.
void rapidcheck(int argc, const char * const *argv);

}

#include "detail/Check.hpp"
