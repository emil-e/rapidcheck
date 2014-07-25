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

// Simplistic generator that we use for stateful testing of Rose. Rose has
// essentially the same semantics as shrink::eachElement so we need some
// generator of a fixed number of value without
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
    prop("generating outside Rose yields the same value as generating "
         "inside it",
         [] (const detail::TestCase &testCase) {
             using namespace detail;

             typedef std::vector<std::string> TestType;
             auto generator = gen::arbitrary<TestType>();

             TestType outsideValue;
             {
                 RandomEngine engine;
                 engine.seed(testCase.seed);
                 ImplicitParam<param::RandomEngine> randomEngine;
                 randomEngine.let(&engine);
                 ImplicitParam<param::Size> size;
                 size.let(testCase.size);
                 ImplicitParam<param::CurrentNode> currentNode;
                 currentNode.let(nullptr);
                 outsideValue = pick(generator);
             }

             Rose<TestType> rose(generator, testCase);
             RC_ASSERT(rose.currentValue() == outsideValue);
         });

    prop("stateful test",
         [] (const detail::TestCase &testCase) {
             auto sizes = pick(
                 gen::collection<std::vector<int>>(gen::ranged(0, 10)));

             SimpleByteGen leafGen;
             typedef decltype(leafGen) LeafGenT;
             typedef VectorGen<LeafGenT> SubGenT;
             std::vector<SubGenT> subGenerators;
             for (int size : sizes) {
                 subGenerators.push_back(
                     VectorGen<LeafGenT>(std::vector<LeafGenT>(size, leafGen)));
             }
             VectorGen<SubGenT> generator(subGenerators);

             detail::Rose<RoseModel::ValueT> rose(generator, testCase);
             RoseModel s0;
             s0.acceptedValue = rose.currentValue();
             s0.currentValue = s0.acceptedValue;
             s0.didShrink = false;
             s0.i1 = 0;
             s0.i2 = 0;
             s0.se = 0;
             state::check(s0, rose, [] (const RoseModel &model) {
                 switch (pick(gen::ranged(0, 3))) {
                 case 0:
                     return state::CommandSP<
                         RoseModel,
                         detail::Rose<RoseModel::ValueT>>(new CurrentValue());

                 case 1:
                     return state::CommandSP<
                         RoseModel,
                         detail::Rose<RoseModel::ValueT>>(new NextShrink());

                 case 2:
                     return state::CommandSP<
                         RoseModel,
                         detail::Rose<RoseModel::ValueT>>(new AcceptShrink());
                 }

                 return state::CommandSP<
                     RoseModel,
                     detail::Rose<RoseModel::ValueT>>(nullptr);
             });
         });
}
