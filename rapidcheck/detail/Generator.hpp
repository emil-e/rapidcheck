#pragma once

#include <vector>

#include "ImplicitParam.hpp"
#include "Rose.hpp"
#include "GenerationParams.hpp"

namespace rc {

template<typename Gen>
typename Gen::GeneratedType pick(Gen generator)
{
    return detail::RoseNode::current().pick(generator);
}

template<typename T>
T pick()
{
    return pick(arbitrary<T>());
}

size_t currentSize()
{
    return *detail::ImplicitParam<detail::param::Size>();
}

template<typename T>
const std::type_info *Generator<T>::generatedTypeInfo() const
{
    return &typeid(T);
}

template<typename T>
std::string Generator<T>::generateString() const
{
    std::ostringstream ss;
    show((*this)(), ss);
    return ss.str();
}

template<typename T>
ShrinkIteratorUP<T> Generator<T>::shrink(T value) const
{
    return ShrinkIteratorUP<T>(new NullIterator<T>());
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

template<typename T>
class Ranged : public Generator<T>
{
public:
    Ranged(T min, T max) : m_min(min), m_max(max) {}

    T operator()() const override
    {
        auto value(pick(noShrink(resize(kReferenceSize, arbitrary<T>()))));
        return m_min + value % (m_max - m_min + 1);
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
        constexpr int n = Multiplexer<Gens...>::numGenerators;
        auto id = pick(resize(kReferenceSize, ranged<int>(0, n - 1)));
        return m_multiplexer.pickWithId(id);
    }

private:
    Multiplexer<Gens...> m_multiplexer;
};

template<typename T>
class NonZero : public Generator<T>
{
public:
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
        Coll coll;
        std::generate_n(std::inserter(coll, coll.end()), length,
                      [&]{ return pick(m_generator); });
        return coll;
    }

    ShrinkIteratorUP<Coll> shrink(Coll value) const override
    {
        return ShrinkIteratorUP<Coll>(
            new RemoveChunksIterator<Coll>(std::move(value)));
    }

private:
    Gen m_generator;
};

template<typename MemberFuncPtr> class FunctorHelper;

template<typename Functor, typename Ret, typename ...Args>
class FunctorHelper<Ret (Functor::*)(Args...) const>
{
public:
    typedef Ret ReturnType;

    explicit FunctorHelper(Functor functor) : m_functor(std::move(functor)) {}

    ReturnType operator()() const
    { return m_functor(pick<typename std::decay<Args>::type>()...); }

private:
    Functor m_functor;
};

template<typename Callable>
class AnyInvocation : public Generator<
    typename FunctorHelper<decltype(&Callable::operator())>::ReturnType>
{
public:
    explicit AnyInvocation(Callable callable) : m_helper(std::move(callable)) {}

    typename FunctorHelper<decltype(&Callable::operator())>::ReturnType
    operator()() const override
    { return m_helper(); }

private:
    FunctorHelper<decltype(&Callable::operator())> m_helper;
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

//
// Factory functions
//

template<typename T>
Arbitrary<T> arbitrary() { return Arbitrary<T>(); }

template<typename Generator, typename Predicate>
SuchThat<Generator, Predicate> suchThat(Generator gen,
                                        Predicate pred)
{ return SuchThat<Generator, Predicate>(std::move(gen), std::move(pred)); }

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
NonZero<T> nonZero() { return NonZero<T>(); }

template<typename Coll, typename Gen>
CollectionGenerator<Coll, Gen> collection(Gen gen)
{ return CollectionGenerator<Coll, Gen>(std::move(gen)); }

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

} // namespace rc


#include "Arbitrary.hpp"
