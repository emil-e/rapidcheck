#pragma once

#include "Quantifier.h"

namespace rc {
namespace detail {

//! Since a property conceptually is a generator of `Result`, this template
//! turn callables into exactly that.
//!
//! TODO better docs
template<typename Testable>
class Property : public gen::Generator<CaseResult>
{
public:
    explicit Property(Testable testable);

    CaseResult generate() const override;
private:

    Quantifier<Testable> m_quantifier;
};

//! Converts `testable` to a property.
template<typename Testable>
Property<Testable> toProperty(Testable testable);

} // namespace detail
} // namespace rc

#include "Property.hpp"
