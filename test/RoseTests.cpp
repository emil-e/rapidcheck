#include <catch.hpp>
#include <rapidcheck.h>

#include "rapidcheck/detail/Results.h"

using namespace rc;

namespace rc {
namespace detail {

void show(const detail::TestCase &testCase, std::ostream &os)
{
    os << "#" << testCase.index << ": seed=" << testCase.seed << ", size="
       << testCase.size;
}

} // namespace detail
} // namespace rc

template<>
class Arbitrary<detail::TestCase> : public gen::Generator<detail::TestCase>
{
public:
    detail::TestCase generate() const override
    {
        detail::TestCase testCase;
        testCase.index = pick<int>();
        testCase.size = pick(gen::ranged<int>(0, gen::currentSize()));
        testCase.seed = pick<decltype(testCase.seed)>();
        return testCase;
    }
};

// Arbitrary integer which gives very erratic values when shrinking.
class ErraticInt : public gen::Generator<int>
{
public:
    int generate() const override
    { return gen::ranged<int>(0, gen::currentSize()).generate(); }

    shrink::IteratorUP<int> shrink(int value) const override
    {
        return shrink::unfold(
            1,
            [] (int x) { return x <= 10; },
            [=] (int x) { return std::make_pair(((value + 2) * x) % 100, x + 1); });
    }
};

// Arbitrary integer which gives very erratic values when shrinking.
class ErraticSum : public gen::Generator<int>
{
public:
    int generate() const override
    {
        int n = pick(ErraticInt());
        int sum = 0;
        for (int i = 0; i < n; i++)
            sum += pick(gen::noShrink(gen::arbitrary<int>()));
        return sum;
    }
};

// Simplistic generator that uses the given generators to generate an
// std::vector
template<typename Gen>
class VectorGen
    : public gen::Generator<std::vector<typename Gen::GeneratedType>>
{
public:
    explicit VectorGen(std::vector<Gen> generators)
        : m_generators(std::move(generators)) {}

    std::vector<typename Gen::GeneratedType> generate() const override
    {
        std::vector<typename Gen::GeneratedType> value;
        for (const auto &gen : m_generators)
            value.push_back(pick(gen));


        return std::move(value);
    }

private:
    std::vector<Gen> m_generators;
};

// Arbitrary<uint8_t> shrinking semantics is not something we like depend on for
// the stateful test so we create another generator with simpler and more
// predictable semantics.
class SimpleByteGen : public gen::Generator<uint8_t>
{
public:
    uint8_t generate() const override
    { return pick(gen::noShrink(gen::arbitrary<uint8_t>())); }

    shrink::IteratorUP<uint8_t> shrink(uint8_t value) const override
    {
        return shrink::unfold(
            0,
            [=] (uint8_t i) { return i != value; },
            [] (uint8_t i) {
                return std::make_pair<uint8_t, uint8_t>(i + 0, i + 1);
            });
    }
};

// So we can have multiple generators of the same type but where some of them
// do not shrink
template<typename Gen>
class OptionalShrink : public gen::Generator<typename Gen::GeneratedType>
{
public:
    typedef typename Gen::GeneratedType T;

    OptionalShrink(Gen generator, bool shrink)
        : m_generator(std::move(generator))
        , m_shrink(shrink) {}

    T generate() const override
    {
        if (m_shrink)
            return m_generator.generate();
        else
            return gen::noShrink(m_generator).generate();
    }

    shrink::IteratorUP<T> shrink(T value) const override
    {
        if (m_shrink)
            return m_generator.shrink(value);
        else
            return shrink::nothing<T>();
    }

private:
    Gen m_generator;
    bool m_shrink;
};

struct RoseModel
{
    typedef std::vector<std::vector<uint8_t>> ValueT;

    ValueT acceptedValue;
    ValueT currentValue;
    int i1;
    int i2;
    uint8_t se;
    bool didShrink;
};

struct CurrentValue
    : public state::Command<RoseModel, detail::Rose<RoseModel::ValueT>>
{
    void run(const RoseModel &s0,
             detail::Rose<RoseModel::ValueT> &rose) const override
    {
        auto value(rose.currentValue());
        RC_ASSERT(value == s0.currentValue);
    }
};

struct NextShrink
    : public state::Command<RoseModel, detail::Rose<RoseModel::ValueT>>
{
    RoseModel nextState(const RoseModel &s0) const override
    {
        RoseModel s1(s0);
        while (s1.i1 < s1.acceptedValue.size())
        {
            if (s1.i2 < s1.acceptedValue[s1.i1].size()) {
                if (s1.se < s1.acceptedValue[s1.i1][s1.i2]) {
                    s1.currentValue = s1.acceptedValue;
                    s1.currentValue[s1.i1][s1.i2] = s1.se;
                    s1.se++;
                    s1.didShrink = true;
                    return s1;
                } else {
                    s1.i2++;
                    s1.se = 0;
                }
            } else {
                s1.i1++;
                s1.i2 = 0;
                s1.se = 0;
            }
        }

        s1.currentValue = s1.acceptedValue;
        s1.didShrink = false;
        return s1;
    }

    void run(const RoseModel &s0,
             detail::Rose<RoseModel::ValueT> &rose) const override
    {
        bool didShrink;
        auto value(rose.nextShrink(didShrink));

        RoseModel s1(nextState(s0));
        RC_ASSERT(value == s1.currentValue);
        RC_ASSERT(didShrink == s1.didShrink);
    }
};

struct AcceptShrink
    : public state::Command<RoseModel, detail::Rose<RoseModel::ValueT>>
{
    RoseModel nextState(const RoseModel &s0) const override
    {
        RC_PRE(s0.didShrink);
        RoseModel s1(s0);
        s1.acceptedValue = s1.currentValue;
        s1.se = 0;
        s1.didShrink = false;
        return s1;
    }

    void run(const RoseModel &s0,
             detail::Rose<RoseModel::ValueT> &rose) const override
    {
        rose.acceptShrink();
    }
};

TEST_CASE("Rose") {
    using namespace detail;

    prop("stateful test",
         [] (const TestCase &testCase) {
             auto sizes = pick(
                 gen::collection<std::vector<std::size_t>>(gen::ranged(0, 10)));

             SimpleByteGen leafGen;
             typedef decltype(leafGen) LeafGenT;
             typedef VectorGen<LeafGenT> SubGenT;
             std::vector<SubGenT> subGenerators;
             for (std::size_t size : sizes) {
                 subGenerators.push_back(
                     VectorGen<LeafGenT>(std::vector<LeafGenT>(size, leafGen)));
             }
             VectorGen<SubGenT> generator(subGenerators);

             RoseModel s0;
             {
                 RandomEngine engine;
                 engine.seed(testCase.seed);
                 ImplicitParam<param::RandomEngine> randomEngine;
                 randomEngine.let(&engine);
                 ImplicitParam<param::Size> size;
                 size.let(testCase.size);
                 ImplicitParam<param::CurrentNode> currentNode;
                 currentNode.let(nullptr);
                 s0.acceptedValue = pick(generator);
                 s0.currentValue = s0.acceptedValue;
                 s0.didShrink = false;
                 s0.i1 = 0;
                 s0.i2 = 0;
                 s0.se = 0;
             }

             Rose<RoseModel::ValueT> rose(generator, testCase);
             state::check(s0, rose, [] (const RoseModel &model) {
                 switch (pick(gen::ranged(0, 3))) {
                 case 0:
                     return state::CommandSP<
                         RoseModel,
                         Rose<RoseModel::ValueT>>(new CurrentValue());

                 case 1:
                     return state::CommandSP<
                         RoseModel,
                         Rose<RoseModel::ValueT>>(new NextShrink());

                 case 2:
                     return state::CommandSP<
                         RoseModel,
                         Rose<RoseModel::ValueT>>(new AcceptShrink());
                 }

                 return state::CommandSP<
                     RoseModel,
                     Rose<RoseModel::ValueT>>(nullptr);
             });
         });

    prop("shrinking of one value does not affect other unrelated values",
         [] (const TestCase &testCase) {
             auto size = pick(gen::ranged<std::size_t>(0, gen::currentSize()));
             std::vector<ErraticSum> generators(size);
             Rose<std::vector<int>> rose(VectorGen<ErraticSum>(generators), testCase);

             bool success = true;
             auto original = rose.currentValue();
             for (int p = 0; p < original.size(); p++) {
                 for (int n = 0; n < 10; n++) {
                     bool didShrink;
                     auto shrink = rose.nextShrink(didShrink);
                     for (int i = 0; i < original.size(); i++) {
                         if (i != p)
                             success = success && (original[i] == shrink[i]);
                     }
                 }
             }

             RC_ASSERT(success);
         });

    prop("honors the NoShrink parameter",
         [] (const TestCase &testCase) {
             auto size = pick(gen::ranged<int>(1, gen::currentSize() + 1));
             auto i = pick(gen::ranged<int>(0, size));
             auto elementGen = gen::scale(0.05, gen::arbitrary<std::vector<uint8_t>>());
             OptionalShrink<decltype(elementGen)> shrinkGen(elementGen, true);
             OptionalShrink<decltype(elementGen)> noShrinkGen(elementGen, false);
             std::vector<decltype(shrinkGen)> generators(size, shrinkGen);
             generators[i] = noShrinkGen;
             VectorGen<decltype(shrinkGen)> generator(generators);

             Rose<typename decltype(generator)::GeneratedType> rose(generator, testCase);
             auto original = rose.currentValue();
             bool didShrink = true;
             while (didShrink)
                 RC_ASSERT(rose.nextShrink(didShrink)[i] == original[i]);
         });
}
