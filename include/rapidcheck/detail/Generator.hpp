#pragma once

#include <vector>
#include <sstream>

#include "rapidcheck/Show.h"
#include "rapidcheck/Shrink.h"

#include "ImplicitParam.h"
#include "Rose.h"
#include "GenerationParams.h"
#include "Quantifier.h"

namespace rc {

template<typename Gen>
typename Gen::GeneratedType pick(Gen generator)
{
    detail::ImplicitParam<detail::param::CurrentNode> currentNode;
    // TODO ugly switching on hasBinding AND nullptr
    if (currentNode.hasBinding() && (*currentNode != nullptr)) {
        return (*currentNode)->pick(
            makeGeneratorUP(std::move(generator)));
    } else {
        return generator();
    }
}

template<typename T>
T pick()
{
    return pick(gen::arbitrary<T>());
}

namespace gen {

template<typename Gen>
void sample(size_t sz, Gen generator)
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
    return ValueDescription((*this)());
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

    typename Gen::GeneratedType operator()() const override
    {
        size_t size = currentSize();
        while (true) { // TODO give up sometime
            auto x(pick(noShrink(resize(size, m_generator))));
            if (m_predicate(x))
                return x;
            size++;
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

    T operator()() const override
    {
        assert(m_max > m_min);
        // TODO this seems a bit broken
        typedef typename std::make_unsigned<T>::type Uint;
        Uint value(pick(noShrink(resize(kReferenceSize, arbitrary<Uint>()))));
        return m_min + value % (m_max - m_min);
    }

private:
    T m_min, m_max;
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

    typename Multiplexer<Gens...>::GeneratedType operator()() const override
    {
        int n = Multiplexer<Gens...>::numGenerators;
        auto id = pick(resize(kReferenceSize, ranged<int>(0, n)));
        return m_multiplexer.pickWithId(id);
    }

private:
    Multiplexer<Gens...> m_multiplexer;
};

// Generators of this form are common, let's not repeat ourselves
#define IMPLEMENT_SUCH_THAT_GEN(GeneratorName, functionName, predicate) \
    template<typename T>                                                \
    class GeneratorName : public Generator<T>                           \
    {                                                                   \
    public:                                                             \
        T operator()() const                                            \
        { return pick(suchThat<T>([](T x) { return (predicate); })); }  \
    };                                                                  \
                                                                        \
    template<typename T>                                                \
    GeneratorName<T> functionName() { return GeneratorName<T>(); }

IMPLEMENT_SUCH_THAT_GEN(NonZero, nonZero, x != 0)
IMPLEMENT_SUCH_THAT_GEN(Positive, positive, x > 0)
IMPLEMENT_SUCH_THAT_GEN(Negative, negative, x < 0)
IMPLEMENT_SUCH_THAT_GEN(NonNegative, nonNegative, x >= 0)

#undef IMPLEMENT_SUCH_THAT_GEN

template<typename Coll, typename Gen>
class Collection : public Generator<Coll>
{
public:
    explicit Collection(Gen generator)
        : m_generator(std::move(generator)) {}

    Coll operator()() const override
    {
        auto length = pick(ranged<typename Coll::size_type>(0, currentSize() + 1));
        auto gen = noShrink(m_generator);
        Coll coll;
        std::generate_n(std::inserter(coll, coll.end()), length,
                      [&]{ return pick(gen); });
        return coll;
    }

    shrink::IteratorUP<Coll> shrink(Coll value) const override
    {
        return shrink::sequentially(
            shrink::removeChunks(value),
            shrink::eachElement(
                value,
                [=](typename Coll::value_type element) {
                    return m_generator.shrink(std::move(element));
                }));
    }

private:
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
    operator()() const override
    { return m_quantifier(); }

private:
    detail::Quantifier<Callable> m_quantifier;
};

template<typename T>
class Constant : public Generator<T>
{
public:
    explicit Constant(T value) : m_value(std::move(value)) {}
    T operator()() const override { return m_value; }

private:
    T m_value;
};

template<typename Gen>
class NoShrink : public Generator<typename Gen::GeneratedType>
{
public:
    explicit NoShrink(Gen generator) : m_generator(std::move(generator)) {}
    typename Gen::GeneratedType operator()() const override
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

    T operator()() const override { return m_mapper(pick(m_generator)); }

private:
    Gen m_generator;
    Mapper m_mapper;
};

template<typename T>
class Character : public Generator<T>
{
public:
    T operator()() const override
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

    typename Gen::GeneratedType operator()() const override
    {
        try {
            return m_generator();
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
    operator()() const override { return m_callable(); }

private:
    Callable m_callable;
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

template<typename Coll, typename Gen>
Collection<Coll, Gen> collection(Gen gen)
{ return Collection<Coll, Gen>(std::move(gen)); }

template<typename Gen>
Resized<Gen> resize(size_t size, Gen gen)
{ return Resized<Gen>(size, std::move(gen)); }

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

template<typename Gen>
GeneratorUP<typename Gen::GeneratedType> makeGeneratorUP(Gen generator)
{
    return GeneratorUP<typename Gen::GeneratedType>(
        new Gen(std::move(generator)));
}

} // namespace gen
} // namespace rc

#include "Arbitrary.hpp"
