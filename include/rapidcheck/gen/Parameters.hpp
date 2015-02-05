#pragma once

#include "Generator.h"

namespace rc {
namespace gen {

template<typename Gen>
class Resize : public Generator<GeneratedT<Gen>>
{
public:
    Resize(int size, Gen generator)
        : m_size(size), m_generator(std::move(generator)) {}

    GeneratedT<Gen> generate() const override
    {
        detail::ImplicitParam<detail::param::Size> size(m_size);
        return m_generator.generate();
    }

    shrink::IteratorUP<GeneratedT<Gen>>
                       shrink(GeneratedT<Gen> value) const override
    { return m_generator.shrink(std::move(value)); }

private:
    int m_size;
    Gen m_generator;
};


template<typename Gen>
class Scale : public Generator<GeneratedT<Gen>>
{
public:
    Scale(double scale, Gen generator)
        : m_scale(scale), m_generator(std::move(generator)) {}

    GeneratedT<Gen> generate() const override
    {
        using namespace rc::detail;
        ImplicitParam<param::Size> size(
            ImplicitParam<param::Size>::value() * m_scale);
        return m_generator.generate();
    }

    shrink::IteratorUP<GeneratedT<Gen>>
    shrink(GeneratedT<Gen> value) const override
    { return m_generator.shrink(std::move(value)); }

private:
    double m_scale;
    Gen m_generator;
};

// First of all overrides param::Shrink so that children do not implicitly
// shrink but since it also does not implement `shrink` which means it
// does not itself shrink
template<typename Gen>
class NoShrink : public Generator<GeneratedT<Gen>>
{
public:
    explicit NoShrink(Gen generator) : m_generator(std::move(generator)) {}
    GeneratedT<Gen> generate() const override
    {
        detail::ImplicitParam<detail::param::NoShrink> noShrink(true);
        return m_generator.generate();
    }

private:
    Gen m_generator;
};

template<typename Gen>
Resize<Gen> resize(int size, Gen gen)
{ return Resize<Gen>(size, std::move(gen)); }

template<typename Gen>
Scale<Gen> scale(double scale, Gen gen)
{ return Scale<Gen>(scale, std::move(gen)); }

template<typename Gen>
NoShrink<Gen> noShrink(Gen generator)
{ return NoShrink<Gen>(std::move(generator)); }

} // namespace gen
} // namespace rc
