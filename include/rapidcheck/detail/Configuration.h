#pragma once

#include "rapidcheck/gen/Generator.h"

namespace rc {
namespace detail {

//! Global suite configuration.
struct Configuration
{
    //! The global seed. The actual seed for a particular test case will be a
    //! hash of this and some specific data which identifies that case.
    RandomEngine::Seed seed = 0;

    //! The number of test cases for each property.
    int defaultMaxSuccess = 100;

    //! The default maximum size for each property.
    int defaultMaxSize = gen::kNominalSize;

    //! The default maximum size for each property.
    int defaultMaxDiscardRatio = gen::kNominalSize;
};

//! Returns the default configuration.
const Configuration &defaultConfiguration();

namespace param {

//! ImplicitParam containing the current configuration.
struct CurrentConfiguration
{
    typedef Configuration ValueType;
    static Configuration defaultValue() { return defaultConfiguration(); }
};

} // namespace param

} // namespace detail
} // namespace rc
