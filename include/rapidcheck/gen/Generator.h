#pragma once

#include <stdexcept>

#include "rapidcheck/shrink/Shrink.h"

namespace rc {

//! Template for generators of arbitrary values of different types. Specialize
//! this template to provide generation for custom types.
//!
//! @tparam T  The generated type.
template<typename T> class Arbitrary;

namespace gen {

//! Thrown to indicate that an appropriate value couldn't be generated.
class GenerationFailure : public std::runtime_error
{
public:
    explicit GenerationFailure(std::string msg);
};

//! Prints a sample value to stdout for the given generator.
//!
//! @param sz         The size to sample.
//! @param generator  The generator.
template<typename Gen>
void sample(int sz, Gen generator);

//! Returns the current size that is being generated.
int currentSize();

//! The reference size. This is not a max limit on the generator size parameter
//! but serves as a guideline. In general, genenerators for which there is a
//! natural limit which is not too expensive to generate should max out at this.
//! This applies to, for example, generation of numbers but not to the
//! generation of collection where there is an associated cost to generating
//! large collections.
constexpr int kNominalSize = 100;

//! Base class for generators of value of type `T`.
template<typename T>
class Generator
{
public:
    //! The generated type.
    typedef T GeneratedType;

    //! Generates a value.
    T operator*() const;

    //! Generates a value. This method should not be called directly, use the
    //! dereferencing operator (i.e. `*someGenerator`) instead. This makes
    //! shrinking work properly.
    virtual T generate() const = 0;

    //! Returns a \c ShrinkIterator which yields the possible shrinks for the
    //! given value. The default impelemtation returns a \c NullIterator.
    virtual shrink::IteratorUP<T> shrink(T value) const;

    ~Generator() = default;

    static_assert(!std::is_same<T, void>::value,
                  "Generated type cannot be void");
};

// Generator implementations
template<typename Gen, typename Predicate> class SuchThat;
template<typename T> class Ranged;
template<typename ...Gens> class OneOf;
template<typename T> class NonZero;
template<typename T> class Positive;
template<typename T> class Negative;
template<typename T> class NonNegative;
template<typename Container, typename Gen> class Vector;
template<typename Container, typename Gen> class Collection;
template<typename Gen> class Resize;
template<typename Gen> class Scale;
template<typename Callable> class AnyInvocation;
template<typename T> class Constant;
template<typename Gen> class NoShrink;
template<typename Gen, typename Mapper> class Mapped;
template<typename T> class Character;
template<typename Exception, typename Gen, typename Catcher> class Rescue;
template<typename Callable> class Lambda;
template<typename ...Gens> class TupleOf;
template<typename Gen1, typename Gen2> class PairOf;

//! Arbitrary generator for type `T`.
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

//! Convenience wrapper for `suchThat(arbitrary<T>, pred)`.
template<typename T, typename Predicate>
SuchThat<Arbitrary<T>, Predicate> suchThat(Predicate pred);

//! Generates an arbitrary value between \c min and \c max. Both \c min and
//! \c max are included in the range.
//!
//! @param min  The minimum value.
//! @param max  The maximum value.
template<typename T>
Ranged<T> ranged(T min, T max);

//! Generates a value by randomly using one of the given generators. All the
//! generators must have the same result type.
template<typename ...Gens>
OneOf<Gens...> oneOf(Gens... generators);

//! Generates a non-zero value of type `T`.
//!
//! @tparam T  An integral type.
template<typename T>
NonZero<T> nonZero();

//! Generates a positive value (> 0) of type `T`.
//!
//! @tparam T  An integral type.
template<typename T>
Positive<T> positive();

//! Generates a negative (< 0) value of type `T`.
//!
//! @tparam T  An integral type.
template<typename T>
Negative<T> negative();

//! Generates a non-negative (>= 0) value of type `T`.
//!
//! @tparam T  An integral type.
template<typename T>
NonNegative<T> nonNegative();

//! Generates a collection of the given size and of type `T` using the given
//! generator.
//!
//! @param size  The size to generate.
//! @param gen   The generator to use.
//!
//! @tparam Container  The collection type.
//! @tparam Gen        The generator type.
template<typename Container, typename Gen>
Vector<Container, Gen> vector(std::size_t size, Gen gen);

//! Generates a collection of the given type using the given generator.
//!
//! @param gen  The generator to use.
//!
//! @tparam Container  The collection type.
//! @tparam Gen        The generator type.
template<typename Container, typename Gen>
Collection<Container, Gen> collection(Gen gen);

//! Returns a version of the given generator that always uses the specified size.
//!
//! @param gen  The generator to wrap.
template<typename Gen>
Resize<Gen> resize(int size, Gen gen);

//! Returns a version of the given generator that scales the generation size
//! according to the given factory.
//!
//! @param gen  The generator to wrap.
template<typename Gen>
Scale<Gen> scale(double scale, Gen gen);

//! Generates values by cwalling the given callable with randomly generated
//! arguments.
template<typename Callable>
AnyInvocation<Callable> anyInvocation(Callable callable);

//! Returns a generator which wraps the given generator but does not try to
//! the argument or any of its sub generators.
template<typename Gen>
NoShrink<Gen> noShrink(Gen generator);

//! Maps a generator of one type to a generator of another type using the given
//! mapping callable.
template<typename Gen, typename Mapper>
Mapped<Gen, Mapper> map(Gen generator, Mapper mapper);

//! Generates a character of type `T`.
//!
//! @tparam T  The character type (i.e. char, wchar_t etc.)
template<typename T>
Character<T> character();

//! Wraps the given generator and catches any exception of type `Exception` and
//! translates them to ordinary return values using a catching function which
//! the exception as an argument and returns the translated value.
template<typename Exception, typename Gen, typename Catcher>
Rescue<Exception, Gen, Catcher> rescue(Gen generator, Catcher catcher);

//! Returns a generator which always generates the same value.
template<typename T>
Constant<T> constant(T value);

//! Creates an anonymous generator that uses the given callable as the
//! generation method.
template<typename Callable>
Lambda<Callable> lambda(Callable callable);

//! Generates tuples containing elements generated by the provided generators.
template<typename ...Gens>
TupleOf<Gens...> tupleOf(Gens ...generators);

//! Generates pairs containing elements generated by the provided generators.
template<typename Gen1, typename Gen2>
PairOf<Gen1, Gen2> pairOf(Gen1 generator1, Gen2 generator2);

} // namespace gen

//! Yields the type generated by the generator `Gen`. The same as
//! `typename Gen::GeneratedType` but shorter.
template<typename Gen>
using GeneratedT = typename Gen::GeneratedType;

namespace detail {

//! Picks a random value using the given generator.
//!
//! @param generator  The Generator to use.
//! @return  The generated value.
template<typename T>
T pick(const gen::Generator<T> &generator);

} // namespace detail
} // namespace rc

#include "Generator.hpp"
