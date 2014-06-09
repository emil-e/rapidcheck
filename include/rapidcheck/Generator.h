#pragma once

#include "Shrink.h"

namespace rc {

//! Picks a random value using the given generator.
//!
//! @param generator  The Generator to use.
//! @return  The generated value.
template<typename Gen>
typename Gen::GeneratedType pick(Gen generator);

//! Picks a random value of the given type. Essentially a shorthand for
//! <pre>pick(arbitrary<T>())</pre>.
//!
//! @tparam T  The type of the value to pick.
template<typename T>
T pick();

//! Template for generators of arbitrary values of different types. Specialize
//! this template to provide generation for custom types.
//!
//! @tparam T  The generated type.
template<typename T> class Arbitrary;

namespace gen {

//! Prints a sample value to stdout for the given generator.
//!
//! @param sz         The size to sample.
//! @param generator  The generator.
template<typename Gen>
void sample(size_t sz, Gen generator);

//! Returns the current size that is being generated.
size_t currentSize();

//! The reference size. This is not a max limit on the generator size parameter
//! but serves as a guideline. In general, genenerators for which there is a
//! natural limit which is not too expensive to generate should max out at this.
//! This applies to, for example, generation of numbers but not to the
//! of collection where there is an associate cost to generating large sizes.
constexpr size_t kReferenceSize = 100;

//! Describes a value and its type.
class ValueDescription
{
public:
    //! Creates a "null" `ValueDescription`.
    ValueDescription() = default;

    template<typename T>
    ValueDescription(const T &value);

    //! Returns the name of the type of this value.
    std::string typeName() const;

    //! Returns a string representation of this value.
    std::string stringValue() const;

    //! Returns `true` if this is a "null" `ValueDescription`.
    bool isNull() const;

private:
    const std::type_info *m_typeInfo = nullptr;
    std::string m_stringValue;
};

//! Base class for generators of all types.
class UntypedGenerator
{
public:
    //! Returns the type info for the generated type.
    virtual const std::type_info &generatedTypeInfo() const = 0;

    //! Generates a value and returns a `ValueDescription` of it. This provides
    //! untyped representation of the value.
    virtual ValueDescription generateDescription() const = 0;

    virtual ~UntypedGenerator() = default;
};

//! Base class for generators of value of type \c T.
//!
//! Note: Instances must be copyable!
template<typename T>
class Generator : public UntypedGenerator
{
public:
    //! The generated type.
    typedef T GeneratedType;

    //! Generates a value.
    virtual T operator()() const = 0;

    //! Returns a \c ShrinkIterator which yields the possible shrinks for the
    //! given value. The default impelemtation returns a \c NullIterator.
    virtual shrink::IteratorUP<T> shrink(T value) const;

    const std::type_info &generatedTypeInfo() const override;
    ValueDescription generateDescription() const override;
};

// Generator implementations
template<typename Gen, typename Predicate> class SuchThat;
template<typename T> class Ranged;
template<typename ...Gens> class OneOf;
template<typename T> class NonZero;
template<typename Coll, typename Gen> class Collection;
template<typename Gen> class Resized;
template<typename Callable> class AnyInvocation;
template<typename T> class Constant;
template<typename Gen> class NoShrink;
template<typename Gen, typename Mapper> class Mapped;
template<typename T> class Character;
template<typename Exception, typename Gen, typename Catcher> class Rescue;

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

//! Generates a value by randomly using one of the given generators. All the
//! generators must have the same result type.
template<typename ...Gens>
OneOf<Gens...> oneOf(Gens... generators);

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
Collection<Coll, Gen> collection(Gen gen);

//! Returns a version of the given generator that always uses the specified size.
//!
//! @param gen  The generator to wrap.
template<typename Gen>
Resized<Gen> resize(size_t size, Gen gen);

//! Generates values by calling the given callable with randomly generated
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

//! Generates a character of type \c T.
//!
//! @tparam T  The character type (i.e. char, wchar_t etc.)
template<typename T>
Character<T> character();

//! Wraps the given generator and catches any exception of type `Exception` and
//! translates them to ordinary return values using a catching function which
//! the exception as an argument and returns the translated value.
template<typename Exception, typename Gen, typename Catcher>
Rescue<Exception, Gen, Catcher> rescue(Gen generator, Catcher catcher);

} // namespace gen
} // namespace rc

#include "detail/Generator.hpp"
