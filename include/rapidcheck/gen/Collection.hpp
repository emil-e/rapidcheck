#pragma once

#include "Generator.h"
#include "Numeric.h"

namespace rc {
namespace gen {

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
            while (!builder.add(*resize(currentSize, gen))) {
                currentSize++;
                if ((currentSize - startSize) > 100) {
                    std::ostringstream msg;
                    msg << "Gave up trying to generate value that can be added";
                    msg << "to container of type '";
                    detail::showType<Container>(msg);
                    msg << "'";
                    throw GenerationFailure(msg.str());
                }
            }
        }
        return std::move(builder.result());
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
        auto size = *ranged<SizeT>(0, currentSize() + 1);
        detail::CollectionBuilder<Container> builder;
        for (int i = 0; i < size; i++)
            builder.add(*noShrink(m_generator));
        return std::move(builder.result());
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

// Specialization for std::array. T must be default constructible.
template<typename T, std::size_t N, typename Gen>
class Collection<std::array<T, N>, Gen> : public Generator<std::array<T, N>>
{
public:
    typedef std::array<T, N> ArrayT;

    static_assert(std::is_default_constructible<T>::value,
                  "T must be default constructible.");

    explicit Collection(Gen generator)
        : m_generator(std::move(generator)) {}

    ArrayT generate() const override
    {
        ArrayT array;
        for (std::size_t i = 0; i < N; i++)
            array[i] = *noShrink(m_generator);
        return std::move(array);
    }

    shrink::IteratorUP<ArrayT> shrink(ArrayT value) const override
    { return shrink(value, detail::IsCopyConstructible<ArrayT>()); }

private:
    shrink::IteratorUP<ArrayT> shrink(const ArrayT &value,
                                      std::false_type) const
    {
        return shrink::nothing<ArrayT>();
    }

    shrink::IteratorUP<ArrayT> shrink(const ArrayT &value,
                                      std::true_type) const
    {
        return shrink::eachElement(
            value,
            [=](typename ArrayT::value_type element) {
                return m_generator.shrink(std::move(element));
            });
    }

    Gen m_generator;
};

template<typename Container, typename Gen>
Vector<Container, Gen> vector(std::size_t size, Gen gen)
{ return Vector<Container, Gen>(size, std::move(gen)); }

template<typename Coll, typename Gen>
Collection<Coll, Gen> collection(Gen gen)
{ return Collection<Coll, Gen>(std::move(gen)); }

} // namespace gen
} // namespace rc
