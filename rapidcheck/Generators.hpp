#pragma once

#include "Arbitrary.hpp"

namespace rc {

//! The reference size. This is not a max limit on the generator size parameter
//! but serves as a guideline. In general, genenerators for which there is a
//! natural limit which is not too expensive to generate should max out at this.
//! This applies to, for example, generation of numbers but not to the
//! of collections where there is an associate cost to generating large sizes.
constexpr size_t kReferenceSize = 100;

//Forward declarations
template<typename T> struct Arbitrary;
template<typename T> Arbitrary<T> arbitrary();
template<typename T> class Ranged;
template<typename T> Ranged<T> ranged(T min, T max);

//! Meta-function for getting the generated type from a generator.
template<typename Generator>
using GeneratedType = typename std::result_of<Generator(size_t)>::type;

template<typename Generator, typename Predicate>
class SuchThat
{
public:
    SuchThat(Generator generator, Predicate predicate)
        : m_generator(std::move(generator))
        , m_predicate(std::move(predicate)) {}

    GeneratedType<Generator> operator()(size_t size) const
    {
        while (true) {
            auto x(m_generator(size));
            if (m_predicate(x))
                return x;
            // Increase size with each try. This prevents problems
            // with, for example, generating non-zero value with a size
            // too small.
            x = m_generator(size);
        }
    }

private:
    Generator m_generator;
    Predicate m_predicate;
};

template<typename T>
class Ranged
{
public:
    Ranged(T min, T max) : m_min(min), m_max(max) {}

    T operator()(size_t size) const
    { return m_min + arbitrary<T>()(kReferenceSize) % (m_max - m_min + 1); }

private:
    T m_min, m_max;
};

template<typename T>
class OneOf
{
public:
    typedef std::function<T(size_t)> Generator;

    OneOf(std::initializer_list<Generator> generators)
        : m_generators(generators) {}

    T operator()(size_t size) const
    {
        typedef typename decltype(m_generators)::size_type SizeType;
        auto index = ranged<SizeType>(0, m_generators.size() - 1)(
            kReferenceSize);
        return m_generators[index](size);
    }
private:
    std::vector<Generator> m_generators;
};

template<typename T>
struct NonZero
{
    T operator()(size_t size) const
    { return suchThat(arbitrary<T>(), [](T x){ return x != 0; })(size); }
};

template<typename Coll, typename Generator>
class CollectionGenerator
{
public:
    explicit CollectionGenerator(Generator generator)
        : m_generator(std::move(generator)) {}

    Coll operator()(size_t size) const
    {
        auto length = ranged<typename Coll::size_type>(0, size)(size);
        Coll coll(length, typename Coll::value_type());
        std::generate(coll.begin(), coll.end(),
                      [&]{ return m_generator(size); });
        return coll;
    }

private:
    Generator m_generator;
};

template<typename Generator>
class Resized
{
public:
    Resized(size_t size, Generator generator)
        : m_size(size), m_generator(std::move(generator)) {}

    GeneratedType<Generator> operator()(size_t size) const
    { return m_generator(m_size); }

private:
    size_t m_size;
    Generator m_generator;
};

//! Arbitrary generator for type \c T. Essentially a wrapper around
//! \c Arbitrary<T> but the naming meshes better with the rest of the generator
//! factories.
//!
//! @tparam T  The generated type.
template<typename T>
Arbitrary<T> arbitrary() { return Arbitrary<T>(); }

//! Uses another generator to generate values satisfying a given condition.
//!
//! @param gen   The underlying generator to use.
//! @param pred  The predicate that the generated values must satisfy
template<typename Generator, typename Predicate>
SuchThat<Generator, Predicate> suchThat(Generator gen,
                                        Predicate pred)
{ return SuchThat<Generator, Predicate>(std::move(gen), std::move(pred)); }

//! Generates an arbitrary value between \c min and \c max. Both \c min and
//! \c max are included in the range.
//!
//! @param min  The minimum value.
//! @param max  The maximum value.
template<typename T>
Ranged<T> ranged(T min, T max)
{
    static_assert(std::is_arithmetic<T>::value,
                  "ranged only supports arithmetic types");
    return Ranged<T>(min, max);
}

//! Generates a value by randomly using one of the given generators.
template<typename Generator, typename ...Generators>
OneOf<GeneratedType<Generator>> oneOf(Generator gen, Generators ...gens)
{
    typedef typename std::result_of<Generator(size_t)>::type T;
    return OneOf<T>{ std::move(gen), std::move(gens)... };
}

//! Generates a non-zero value of type \c T.
//!
//! @tparam T  An integral type.
template<typename T>
NonZero<T> nonZero() { return NonZero<T>(); }

//! Generates a collection of the given type using the given generator.
//!
//! @param gen  The generator to use.
//!
//! @tparam C          The collection type.
//! @tparam Generator  The generator type.
template<typename Coll, typename Generator>
CollectionGenerator<Coll, Generator> collection(Generator gen)
{ return CollectionGenerator<Coll, Generator>(std::move(gen)); }

//! Returns a version of the given generator that always uses the specified size.
//!
//! @param gen  The generator to wrap.
template<typename Generator>
Resized<Generator> resize(size_t size, Generator gen)
{ return Resized<Generator>(size, std::move(gen)); }

}
