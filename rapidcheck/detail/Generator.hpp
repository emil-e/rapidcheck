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
ShrinkIteratorUP<T> Generator<T>::shrink(const T &value) const
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
        auto value(pick(noShrink(resize(kReferenceSize, arbitrary<T>()))));
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
        Coll coll(length, typename Coll::value_type());
        std::generate(coll.begin(), coll.end(),
                      [&]{ return pick(m_generator); });
        return coll;
    }

    ShrinkIteratorUP<Coll> shrink(const Coll &value) const override
    {
        return ShrinkIteratorUP<Coll>(new RemoveChunksIterator<Coll>(value));
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
    explicit Constant(const T &value) : m_value(value) {}
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
        return m_generator();
    }

private:
    Gen m_generator;
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

template<typename Gen, typename ...Gens>
OneOf<typename Gen::GeneratedType> oneOf(Gen gen, Gens ...gens)
{
    return OneOf<typename Gen::GeneratedType>{
        GeneratorUP<typename Gen::GeneratedType>(new Gen(std::move(gen))),
        GeneratorUP<typename Gen::GeneratedType>(new Gens(std::move(gens)))...
    };
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

} // namespace rc

#include "Arbitrary.hpp"
