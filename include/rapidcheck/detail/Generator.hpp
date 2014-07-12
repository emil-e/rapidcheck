#pragma once

#include <vector>
#include <sstream>

#include "rapidcheck/Show.h"
#include "rapidcheck/Shrink.h"

#include "ImplicitParam.h"
#include "Rose.h"
#include "GenerationParams.h"
#include "Quantifier.h"
#include "CollectionBuilder.h"

namespace rc {

template<typename Gen>
typename Gen::GeneratedType pick(Gen generator)
{
    detail::ImplicitParam<detail::param::CurrentNode> currentNode;
    // TODO ugly switching on hasBinding AND nullptr
    if (currentNode.hasBinding() && (*currentNode != nullptr)) {
        return (*currentNode)->pick(
            gen::GeneratorUP<typename Gen::GeneratedType>(
                new Gen(std::move(generator))));
    } else {
        return generator.generate();
    }
}

template<typename T>
T pick()
{
    return pick(gen::arbitrary<T>());
}

namespace gen {

template<typename Gen>
void sample(int sz, Gen generator)
{
    using namespace detail;

    ImplicitParam<param::Size> size;
    size.let(sz);

    ImplicitParam<param::RandomEngine> randomEngine;
    randomEngine.let(RandomEngine());

    show(generator(), std::cout);
    std::cout << std::endl;
}

template<typename T>
ValueDescription::ValueDescription(const T &value)
    : m_typeInfo(&typeid(T))
{
    std::ostringstream ss;
    show(value, ss);
    m_stringValue = ss.str();
}

template<typename T>
const std::type_info &Generator<T>::generatedTypeInfo() const
{
    return typeid(T);
}

template<typename T>
ValueDescription Generator<T>::generateDescription() const
{
    return ValueDescription(generate());
}

template<typename T>
shrink::IteratorUP<T> Generator<T>::shrink(T value) const
{
    return shrink::nothing<T>();
}

template<typename Gen, typename Predicate>
class SuchThat : public Generator<typename Gen::GeneratedType>
{
public:
    SuchThat(Gen generator, Predicate predicate)
        : m_generator(std::move(generator))
        , m_predicate(std::move(predicate)) {}

    typename Gen::GeneratedType generate() const override
    {
        auto startSize = currentSize();
        auto size = startSize;
        while (true) { // TODO give up sometime
            auto x(pick(noShrink(resize(size, m_generator))));
            if (m_predicate(x))
                return x;
            size++;
            if ((size - startSize) > 100) {
                throw GenerationFailure(
                    "Gave up trying to generate value satisfying predicate");
            }
        }
    }

private:
    Gen m_generator;
    Predicate m_predicate;
};

// TODO make shrinkable?
template<typename T>
class Ranged : public Generator<T>
{
public:
    Ranged(T min, T max) : m_min(min), m_max(max) {}

    T generate() const override
    {
        assert(m_max >= m_min);
        if (m_max == m_min)
            return m_max;

        // TODO this seems a bit broken
        typedef typename std::make_unsigned<T>::type Uint;
        Uint value(pick(noShrink(resize(kReferenceSize, arbitrary<Uint>()))));
        return m_min + value % (m_max - m_min);
    }

private:
    T m_min, m_max;
};

template<typename Gen>
class Resize : public Generator<typename Gen::GeneratedType>
{
public:
    Resize(int size, Gen generator)
        : m_size(size), m_generator(std::move(generator)) {}

    typename Gen::GeneratedType generate() const override
    {
        detail::ImplicitParam<detail::param::Size> size;
        size.let(m_size);
        return m_generator.generate();
    }

    shrink::IteratorUP<typename Gen::GeneratedType>
    shrink(typename Gen::GeneratedType value) const override
    { return m_generator.shrink(std::move(value)); }

private:
    int m_size;
    Gen m_generator;
};


template<typename Gen>
class Scale : public Generator<typename Gen::GeneratedType>
{
public:
    Scale(double scale, Gen generator)
        : m_scale(scale), m_generator(std::move(generator)) {}

    typename Gen::GeneratedType generate() const override
    {
        detail::ImplicitParam<detail::param::Size> size;
        size.let(*size * m_scale);
        return m_generator.generate();
    }

    shrink::IteratorUP<typename Gen::GeneratedType>
    shrink(typename Gen::GeneratedType value) const override
    { return m_generator.shrink(std::move(value)); }

private:
    double m_scale;
    Gen m_generator;
};


// Helper class for OneOf to be able to have a collection of generators of
// different types
template<typename ...Gens>
class Multiplexer;

template<typename Gen, typename ...Gens>
class Multiplexer<Gen, Gens...>
{
public:
    typedef typename Gen::GeneratedType GeneratedType;
    static constexpr int numGenerators = sizeof...(Gens) + 1;

    static_assert(
        std::is_same<
            typename Gen::GeneratedType,
            typename std::tuple_element<0,
                std::tuple<typename Gens::GeneratedType...>>::type>::value,
        "All generators must have the same result type");

    Multiplexer(Gen generator, Gens... generators)
        : m_generator(std::move(generator))
        , m_multiplexer(std::move(generators)...) {}

    typename Gen::GeneratedType pickWithId(int id) const
    {
        if (id == myId)
            return pick(m_generator);
        else
            return m_multiplexer.pickWithId(id);
    }

private:
    static constexpr int myId = sizeof...(Gens);

    Gen m_generator;
    Multiplexer<Gens...> m_multiplexer;
};

template<typename Gen>
class Multiplexer<Gen>
{
public:
    typedef typename Gen::GeneratedType GeneratedType;
    static constexpr int numGenerators = 1;

    Multiplexer(Gen generator)
        : m_generator(std::move(generator)) {}

    typename Gen::GeneratedType pickWithId(int id) const
    { return pick(m_generator); }

private:
    static constexpr int myId = 0;

    Gen m_generator;
};

template<typename ...Gens>
class OneOf : public Generator<typename Multiplexer<Gens...>::GeneratedType>
{
public:
    OneOf(Gens... generators) : m_multiplexer(std::move(generators)...) {}

    typename Multiplexer<Gens...>::GeneratedType generate() const override
    {
        int n = Multiplexer<Gens...>::numGenerators;
        auto id = pick(resize(kReferenceSize, ranged<int>(0, n)));
        return m_multiplexer.pickWithId(id);
    }

private:
    Multiplexer<Gens...> m_multiplexer;
};

// Generators of this form are common, let's not repeat ourselves
#define IMPLEMENT_SUCH_THAT_GEN(GeneratorName, predicate)               \
    template<typename T>                                                \
    class GeneratorName : public Generator<T>                           \
    {                                                                   \
    public:                                                             \
        T generate() const                                            \
        { return pick(suchThat<T>([](T x) { return (predicate); })); }  \
    };

IMPLEMENT_SUCH_THAT_GEN(NonZero, x != 0)
IMPLEMENT_SUCH_THAT_GEN(Positive, x > 0)
IMPLEMENT_SUCH_THAT_GEN(Negative, x < 0)
IMPLEMENT_SUCH_THAT_GEN(NonNegative, x >= 0)

#undef IMPLEMENT_SUCH_THAT_GEN

template<typename Container, typename Gen>
class Vector : public Generator<Container>
{
public:
    explicit Vector(std::size_t size, Gen generator)
        : m_size(size)
        , m_generator(std::move(generator)) {}

    Container generate() const override
    {
        detail::CollectionBuilder<Container> builder;
        auto gen = noShrink(m_generator);
        for (int i = 0; i < m_size; i++) {
            // Gradually increase size for every failed adding attempt to
            // increase likelihood that we get a "successful" value the next
            // time
            auto startSize = gen::currentSize();
            auto currentSize = startSize;
            while (!builder.add(pick(resize(currentSize, gen)))) {
                currentSize++;
                if ((currentSize - startSize) > 100) {
                    std::string msg;
                    msg += "Gave up trying to generate value that can be added to ";
                    msg += "container of type '";
                    msg += detail::demangle(typeid(Container).name()) + "'";
                    throw GenerationFailure(msg);
                }
            }
        }
        return std::move(builder.collection());
    }

    shrink::IteratorUP<Container> shrink(Container value) const override
    { return shrink(value, detail::IsCopyConstructible<Container>()); }

private:
    shrink::IteratorUP<Container> shrink(const Container &value,
                                         std::false_type) const
    {
        return shrink::nothing<Container>();
    }

    shrink::IteratorUP<Container> shrink(const Container &value,
                                         std::true_type) const
    {
        return shrink::eachElement(
            value,
            [=](typename Container::value_type element) {
                return m_generator.shrink(std::move(element));
            });
    }

    std::size_t m_size;
    Gen m_generator;
};

template<typename Container, typename Gen>
class Collection : public Generator<Container>
{
public:
    explicit Collection(Gen generator)
        : m_generator(std::move(generator)) {}

    Container generate() const override
    {
        typedef typename Container::size_type SizeT;
        auto size = pick(ranged<SizeT>(0, currentSize() + 1));
        detail::CollectionBuilder<Container> builder;
        for (int i = 0; i < size; i++)
            builder.add(pick(noShrink(m_generator)));
        return std::move(builder.collection());
    }

    shrink::IteratorUP<Container> shrink(Container value) const override
    { return shrink(value, detail::IsCopyConstructible<Container>()); }

private:
    shrink::IteratorUP<Container> shrink(const Container &value,
                                         std::false_type) const
    {
        return shrink::nothing<Container>();
    }

    shrink::IteratorUP<Container> shrink(const Container &value,
                                         std::true_type) const
    {
        return shrink::sequentially(
            shrink::removeChunks(value),
            shrink::eachElement(
                value,
                [=](typename Container::value_type element) {
                    return m_generator.shrink(std::move(element));
                }));
    }

    Gen m_generator;
};

template<typename Callable>
class AnyInvocation : public Generator<
    typename detail::Quantifier<Callable>::ReturnType>
{
public:
    explicit AnyInvocation(Callable callable)
        : m_quantifier(std::move(callable)) {}

    typename detail::Quantifier<Callable>::ReturnType
    generate() const override
    { return m_quantifier(); }

private:
    detail::Quantifier<Callable> m_quantifier;
};

template<typename T>
class Constant : public Generator<T>
{
public:
    explicit Constant(T value) : m_value(std::move(value)) {}
    T generate() const override { return m_value; }

private:
    T m_value;
};

template<typename Gen>
class NoShrink : public Generator<typename Gen::GeneratedType>
{
public:
    explicit NoShrink(Gen generator) : m_generator(std::move(generator)) {}
    typename Gen::GeneratedType generate() const override
    {
        detail::ImplicitParam<detail::param::NoShrink> noShrink;
        noShrink.let(true);
        return pick(m_generator);
    }

private:
    Gen m_generator;
};

template<typename Gen, typename Mapper>
class Mapped : public Generator<
    typename std::result_of<Mapper(typename Gen::GeneratedType)>::type>
{
public:
    typedef typename
        std::result_of<Mapper(typename Gen::GeneratedType)>::type T;

    Mapped(Gen generator, Mapper mapper)
        : m_generator(std::move(generator))
        , m_mapper(std::move(mapper)) {}

    T generate() const override { return m_mapper(pick(m_generator)); }

private:
    Gen m_generator;
    Mapper m_mapper;
};

template<typename T>
class Character : public Generator<T>
{
public:
    T generate() const override
    {
        return pick(oneOf(map(ranged<uint8_t>(1, 128),
                              [](uint8_t x) { return static_cast<T>(x); }),
                          nonZero<T>()));
    }

    shrink::IteratorUP<T> shrink(T value) const override
    {
        // TODO this can probably be better
        std::vector<T> chars;
        switch (value) {
        default:
            chars.insert(chars.begin(), static_cast<T>('3'));
        case '3':
            chars.insert(chars.begin(), static_cast<T>('2'));
        case '2':
            chars.insert(chars.begin(), static_cast<T>('1'));
        case '1':
            chars.insert(chars.begin(), static_cast<T>('C'));
        case 'C':
            chars.insert(chars.begin(), static_cast<T>('B'));
        case 'B':
            chars.insert(chars.begin(), static_cast<T>('A'));
        case 'A':
            chars.insert(chars.begin(), static_cast<T>('c'));
        case 'c':
            chars.insert(chars.begin(), static_cast<T>('b'));
        case 'b':
            chars.insert(chars.begin(), static_cast<T>('a'));
        case 'a':
            ;
        }

        return shrink::constant<T>(chars);
    }
};

template<typename Exception, typename Gen, typename Catcher>
class Rescue : public Generator<typename Gen::GeneratedType>
{
public:
    Rescue(Gen generator, Catcher catcher)
        : m_generator(generator), m_catcher(catcher) {}

    typename Gen::GeneratedType generate() const override
    {
        try {
            return m_generator.generate();
        } catch (const Exception &e) {
            return m_catcher(e);
        }
    }

private:
    Gen m_generator;
    Catcher m_catcher;
};

template<typename Callable>
class Lambda : public Generator<typename std::result_of<Callable()>::type>
{
public:
    explicit Lambda(Callable callable) : m_callable(std::move(callable)) {}

    typename std::result_of<Callable()>::type
    generate() const override { return m_callable(); }

private:
    Callable m_callable;
};

template<typename ...Gens>
class TupleOf;

template<>
class TupleOf<> : public Generator<std::tuple<>>
{
public:
    std::tuple<> generate() const override { return std::tuple<>(); }
};

#define IMPLEMENT_CONDITIONAL

template<typename Gen, typename ...Gens>
class TupleOf<Gen, Gens...>
    : public Generator<std::tuple<typename Gen::GeneratedType,
                                  typename Gens::GeneratedType...>>
{
public:
    typedef std::tuple<typename Gen::GeneratedType,
                       typename Gens::GeneratedType...> TupleT;
    typedef typename Gen::GeneratedType HeadT;
    typedef std::tuple<typename Gens::GeneratedType...> TailT;

    TupleOf(Gen headGenerator, Gens ...tailGenerators)
        : m_headGenerator(std::move(headGenerator))
        , m_tailGenerator(std::move(tailGenerators)...) {}

    TupleT generate() const override
    {
        return std::tuple_cat(
            std::tuple<typename Gen::GeneratedType>(pick(m_headGenerator)),
            pick(m_tailGenerator));
    }

    shrink::IteratorUP<TupleT> shrink(TupleT value) const override
    { return shrink(value, detail::IsCopyConstructible<TupleT>()); }

private:
    shrink::IteratorUP<TupleT> shrink(const TupleT &value,
                                      std::false_type) const
    { return shrink::nothing<TupleT>(); }

    shrink::IteratorUP<TupleT> shrink(const TupleT &value,
                                      std::true_type) const
    {
        // Shrink the head and map it by append the unshrunk tail,
        // then shrink the tail and map it by prepending the unshrink head.
        return shrink::sequentially(
            shrink::map(m_headGenerator.shrink(std::get<0>(value)),
                        [=] (HeadT &&x) -> TupleT {
                            return std::tuple_cat(
                                std::tuple<HeadT>(std::move(x)),
                                detail::tupleTail(value));
                        }),
            shrink::map(m_tailGenerator.shrink(detail::tupleTail(value)),
                        [=] (TailT &&x) -> TupleT {
                            return std::tuple_cat(
                                std::tuple<HeadT>(std::get<0>(value)),
                                std::move(x));
                        }));
    }

    Gen m_headGenerator;
    TupleOf<Gens...> m_tailGenerator;
};

template<typename Gen1, typename Gen2>
class PairOf : public Generator<std::pair<typename Gen1::GeneratedType,
                                          typename Gen2::GeneratedType>>
{
public:
    typedef typename Gen1::GeneratedType T1;
    typedef typename Gen2::GeneratedType T2;
    typedef typename std::pair<T1, T2> PairT;

    PairOf(Gen1 generator1, Gen2 generator2)
        : m_generator(std::move(generator1),
                      std::move(generator2)) {}

    PairT generate() const override
    {
        auto x = m_generator.generate();
        return PairT(std::move(std::get<0>(x)),
                     std::move(std::get<1>(x)));
    }

    shrink::IteratorUP<PairT> shrink(PairT value) const override
    {
        return shrink::map(
            m_generator.shrink(std::tuple<T1, T2>(std::move(value.first),
                                                  std::move(value.second))),
            [] (std::tuple<T1, T2> &&x) {
                return PairT(std::move(std::get<0>(x)),
                             std::move(std::get<1>(x)));
            });
    }

private:
    TupleOf<Gen1, Gen2> m_generator;
};

//
// Factory functions
//

template<typename T>
Arbitrary<T> arbitrary() { return Arbitrary<T>(); }

template<typename Generator, typename Predicate>
SuchThat<Generator, Predicate> suchThat(Generator gen,
                                        Predicate pred)
{ return SuchThat<Generator, Predicate>(std::move(gen), std::move(pred)); }

template<typename T, typename Predicate>
SuchThat<Arbitrary<T>, Predicate> suchThat(Predicate pred)
{ return suchThat(arbitrary<T>(), std::move(pred)); }

template<typename T>
Ranged<T> ranged(T min, T max)
{
    static_assert(std::is_arithmetic<T>::value,
                  "ranged only supports arithmetic types");
    return Ranged<T>(min, max);
}

template<typename ...Gens>
OneOf<Gens...> oneOf(Gens... generators)
{
    return OneOf<Gens...>(std::move(generators)...);
}

template<typename T>
NonZero<T> nonZero()
{ return NonZero<T>(); }

template<typename T>
Positive<T> positive()
{ return Positive<T>(); }

template<typename T>
Negative<T> negative()
{
    static_assert(std::is_signed<T>::value,
                  "gen::negative can only be used for signed types");
    return Negative<T>();
}

template<typename T>
NonNegative<T> nonNegative()
{ return NonNegative<T>(); }

template<typename Container, typename Gen>
Vector<Container, Gen> vector(std::size_t size, Gen gen)
{ return Vector<Container, Gen>(size, std::move(gen)); }

template<typename Coll, typename Gen>
Collection<Coll, Gen> collection(Gen gen)
{ return Collection<Coll, Gen>(std::move(gen)); }

template<typename Gen>
Resize<Gen> resize(int size, Gen gen)
{ return Resize<Gen>(size, std::move(gen)); }

template<typename Gen>
Scale<Gen> scale(double scale, Gen gen)
{ return Scale<Gen>(scale, std::move(gen)); }

template<typename Callable>
AnyInvocation<Callable> anyInvocation(Callable callable)
{ return AnyInvocation<Callable>(std::move(callable)); }

template<typename Gen>
NoShrink<Gen> noShrink(Gen generator)
{ return NoShrink<Gen>(std::move(generator)); }

template<typename Gen, typename Mapper>
Mapped<Gen, Mapper> map(Gen generator, Mapper mapper)
{ return Mapped<Gen, Mapper>(std::move(generator), std::move(mapper)); }

template<typename T>
Character<T> character() { return Character<T>(); }

// TODO deduce exception type
template<typename Exception, typename Gen, typename Catcher>
Rescue<Exception, Gen, Catcher> rescue(Gen generator, Catcher catcher)
{
    return Rescue<Exception, Gen, Catcher>(std::move(generator),
                                           std::move(catcher));
}

template<typename T>
Constant<T> constant(T value) { return Constant<T>(std::move(value)); }

template<typename Callable>
Lambda<Callable> lambda(Callable callable)
{ return Lambda<Callable>(std::move(callable)); }

template<typename ...Gens>
TupleOf<Gens...> tupleOf(Gens ...generators)
{ return TupleOf<Gens...>(std::move(generators)...); }

template<typename Gen1, typename Gen2>
PairOf<Gen1, Gen2> pairOf(Gen1 generator1, Gen2 generator2)
{ return PairOf<Gen1, Gen2>(std::move(generator1), std::move(generator2)); }

} // namespace gen
} // namespace rc

#include "Arbitrary.hpp"
