#pragma once

#include <vector>

#include "Arbitrary.hpp"
#include "ImplicitParam.hpp"

namespace rc {

namespace detail {
namespace param {

//! The current generation size.
struct Size { typedef size_t ValueType; };

} // namespace param
} // namespace detail

//! The reference size. This is not a max limit on the generator size parameter
//! but serves as a guideline. In general, genenerators for which there is a
//! natural limit which is not too expensive to generate should max out at this.
//! This applies to, for example, generation of numbers but not to the
//! of collection where there is an associate cost to generating large sizes.
constexpr size_t kReferenceSize = 100;

//Forward declarations
template<typename T> class Arbitrary;
template<typename T> Arbitrary<T> arbitrary();
template<typename T> class Ranged;
template<typename T> Ranged<T> ranged(T min, T max);

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

//! unique_ptr to \c Generator<T>.
template<typename T>
using GeneratorUP = std::unique_ptr<Generator<T>>;

//! Returns the current size that is being generated.
size_t currentSize()
{
    return *detail::ImplicitParam<detail::param::Size>();
}

template<typename Gen, typename Predicate>
class SuchThat : public Generator<typename Gen::GeneratedType>
{
public:
    SuchThat(Gen generator, Predicate predicate)
        : m_generator(std::move(generator))
        , m_predicate(std::move(predicate)) {}

    typename Gen::GeneratedType operator()() const override
    {
        size_t size = currentSize();
        while (true) {
            auto x(pick(resize(size, m_generator)));
            if (m_predicate(x))
                return x;
            size++;
        }
    }

private:
    Gen m_generator;
    Predicate m_predicate;
};

template<typename T>
class Ranged : public Generator<T>
{
public:
    Ranged(T min, T max) : m_min(min), m_max(max) {}

    T operator()() const override
    {
        auto value(pick(resize(kReferenceSize, arbitrary<T>())));
        return m_min + value % (m_max - m_min + 1);
    }

private:
    T m_min, m_max;
};

template<typename T>
class OneOf : public Generator<T>
{
public:
    OneOf(std::initializer_list<GeneratorUP<T>> generators)
        : m_generators(generators)
    {
    }

    T operator()() const override
    {
        typedef typename decltype(m_generators)::size_type SizeType;
        auto index = pick(
            resize(kReferenceSize,
                   ranged<SizeType>(0, m_generators.size() - 1)));
        return pick(*m_generators[index]);
    }
private:
    std::vector<GeneratorUP<T>> m_generators;
};

template<typename T>
struct NonZero : public Generator<T>
{
    T operator()() const
    { return pick(suchThat(arbitrary<T>(), [](T x){ return x != 0; })); }
};

template<typename Coll, typename Gen>
class CollectionGenerator : public Generator<Coll>
{
public:
    explicit CollectionGenerator(Gen generator)
        : m_generator(std::move(generator)) {}

    Coll operator()() const override
    {
        auto length = pick(ranged<typename Coll::size_type>(0, currentSize()));
        Coll coll(length, typename Coll::value_type());
        std::generate(coll.begin(), coll.end(),
                      [&]{ return pick(m_generator); });
        return coll;
    }

private:
    Gen m_generator;
};

template<typename Gen>
class Resized : public Generator<typename Gen::GeneratedType>
{
public:
    Resized(size_t size, Gen generator)
        : m_size(size), m_generator(std::move(generator)) {}

    typename Gen::GeneratedType operator()() const override
    {
        detail::ImplicitParam<detail::param::Size> size;
        size.let(m_size);
        return pick(m_generator);
    }

private:
    size_t m_size;
    Gen m_generator;
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
template<typename Gen, typename ...Gens>
OneOf<typename Gen::GeneratedType> oneOf(Gen gen, Gens ...gens)
{
    return OneOf<typename Gen::GeneratedType>{
        GeneratorUP<typename Gen::GeneratedType>(new Gen(std::move(gen))),
        GeneratorUP<typename Gen::GeneratedType>(new Gens(std::move(gens)))...
    };
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
template<typename Coll, typename Gen>
CollectionGenerator<Coll, Gen> collection(Gen gen)
{ return CollectionGenerator<Coll, Gen>(std::move(gen)); }

//! Returns a version of the given generator that always uses the specified size.
//!
//! @param gen  The generator to wrap.
template<typename Gen>
Resized<Gen> resize(size_t size, Gen gen)
{ return Resized<Gen>(size, std::move(gen)); }

} // namespace rc
