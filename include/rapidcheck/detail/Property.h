#pragma ocne

#include "Quantifier.h"

namespace rc {
namespace detail {

//! Since a property conceptually is a generator of `Result`, this template
//! turn callables into exactly that.
//!
//! TODO better docs
template<typename Testable>
class Property : public gen::Generator<Result>
{
public:
    explicit Property(Testable testable);

    Result operator()() const override;
private:

    Quantifier<Testable> m_quantifier;
};

//! Converts `testable` to a property.
template<typename Testable>
gen::GeneratorUP<Result> toProperty(Testable testable);

} // namespace detail
} // namespace rc

#include "Property.hpp"
