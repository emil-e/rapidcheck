#pragma once

namespace rc {

//! Picks a random value using the given generator.
//!
//! @param generator  The Generator to use.
//! @return  The generated value.
template<typename Gen>
typename Gen::GeneratedType pick(const Gen &generator);

//! Picks a random value of the given type. Essentially a shorthand for
//! <pre>pick(arbitrary<T>())</pre>.
//!
//! @tparam T  The type of the value to pick.
template<typename T>
T pick();

//! The reference size. This is not a max limit on the generator size parameter
//! but serves as a guideline. In general, genenerators for which there is a
//! natural limit which is not too expensive to generate should max out at this.
//! This applies to, for example, generation of numbers but not to the
//! of collection where there is an associate cost to generating large sizes.
constexpr size_t kReferenceSize = 100;

//! Base class for generators. Parameterized on the generated type \c T.
template<typename T>
class Generator
{
public:
    //! The generated type.
    typedef T GeneratedType;

    //! Generates a value.
    virtual T operator()() const = 0;
};

//! \c std::unique_ptr to \c Generator<T>.
template<typename T>
using GeneratorUP = std::unique_ptr<Generator<T>>;

//! Returns the current size that is being generated.
size_t currentSize();

// Generator implementations
template<typename Gen, typename Predicate> class SuchThat;
template<typename T> class Ranged;
template<typename T> class OneOf;
template<typename T> class NonZero;
template<typename Coll, typename Gen> class CollectionGenerator;
template<typename Gen> class Resized;
template<typename Callable> class AnyInvocation;

//! Template for generators of arbitrary values of different types. Specialize
//! this template to provide generation for custom types.
//!
//! @tparam T  The generated type.
template<typename T> class Arbitrary;

//! Arbitrary generator for type \c T.
//!
//! @tparam T  The generated type.
template<typename T>
Arbitrary<T> arbitrary();

//! Uses another generator to generate values satisfying a given condition.
//!
//! @param gen   The underlying generator to use.
//! @param pred  The predicate that the generated values must satisfy
template<typename Generator, typename Predicate>
SuchThat<Generator, Predicate> suchThat(Generator gen, Predicate pred);

//! Generates an arbitrary value between \c min and \c max. Both \c min and
//! \c max are included in the range.
//!
//! @param min  The minimum value.
//! @param max  The maximum value.
template<typename T>
Ranged<T> ranged(T min, T max);

//! Generates a value by randomly using one of the given generators.
template<typename Gen, typename ...Gens>
OneOf<typename Gen::GeneratedType> oneOf(Gen gen, Gens ...gens);

//! Generates a non-zero value of type \c T.
//!
//! @tparam T  An integral type.
template<typename T>
NonZero<T> nonZero();

//! Generates a collection of the given type using the given generator.
//!
//! @param gen  The generator to use.
//!
//! @tparam C          The collection type.
//! @tparam Generator  The generator type.
template<typename Coll, typename Gen>
CollectionGenerator<Coll, Gen> collection(Gen gen);

//! Returns a version of the given generator that always uses the specified size.
//!
//! @param gen  The generator to wrap.
template<typename Gen>
Resized<Gen> resize(size_t size, Gen gen);

//! Generates values by calling the given callable with randomly generated
//! arguments.
template<typename Callable>
AnyInvocation<Callable> anyInvocation(Callable callable);

} // namespace rc

#include "detail/Generator.hpp"
