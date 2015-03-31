#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/Generators.h"

#include "rapidcheck/gen/Generator.h"
#include "rapidcheck/detail/Results.h"
#include "rapidcheck/detail/Rose.h"
#include "rapidcheck/detail/ShowType.h"

using namespace rc;
using namespace rc::detail;

namespace {

// Arbitrary integer which gives very erratic values when shrinking.
class ErraticInt : public gen::Generator<int>
{
public:
    int generate() const override
    { return gen::ranged<int>(0, gen::currentSize() + 1).generate(); }

    Seq<int> shrink(int value) const override
    {
        return seq::map(seq::range(0, 10), [=](int x) {
            return ((value + 2) * (x + 1)) % 100;
        });
    }
};

// Arbitrary integer which gives very erratic values when shrinking.
class ErraticSum : public gen::Generator<int>
{
public:
    int generate() const override
    {
        int n = *ErraticInt();
        int sum = 0;
        for (int i = 0; i < n; i++)
            sum += *gen::noShrink(gen::arbitrary<int>());
        return sum;
    }
};

// Simplistic generator that uses the given generators to generate an
// std::vector
template<typename Gen>
class VectorGen
    : public gen::Generator<std::vector<gen::GeneratedT<Gen>>>
{
public:
    explicit VectorGen(std::vector<Gen> generators)
        : m_generators(std::move(generators)) {}

    std::vector<gen::GeneratedT<Gen>> generate() const override
    {
        std::vector<gen::GeneratedT<Gen>> value;
        for (const auto &gen : m_generators)
            value.push_back(*gen);


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
    { return *gen::noShrink(gen::arbitrary<uint8_t>()); }

    Seq<uint8_t> shrink(uint8_t value) const override
    { return seq::range<uint8_t>(0, value); }
};

// So we can have multiple generators of the same type but where some of them
// do not shrink
template<typename Gen>
class OptionalShrink : public gen::Generator<gen::GeneratedT<Gen>>
{
public:
    typedef gen::GeneratedT<Gen> T;

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

    Seq<T> shrink(T value) const override
    {
        if (m_shrink)
            return m_generator.shrink(value);
        else
            return Seq<T>();
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

typedef state::Command<RoseModel, Rose<RoseModel::ValueT>> RoseCommand;
typedef std::shared_ptr<RoseCommand> RoseCommandSP;

struct CurrentValue : public RoseCommand
{
    void run(const RoseModel &s0, Rose<RoseModel::ValueT> &rose) const override
    {
        auto value(rose.currentValue());
        RC_ASSERT(value == s0.currentValue);
    }
};

struct GetExample : public RoseCommand
{
    void run(const RoseModel &s0, Rose<RoseModel::ValueT> &rose) const override
    {
        auto example(rose.example());
        std::vector<std::pair<std::string, std::string>> expected;
        for (const auto &x : s0.currentValue) {
            expected.emplace_back(
                typeToString<RoseModel::ValueT::value_type>(),
                toString(x));
        }
        RC_ASSERT(example == expected);
    }
};

struct NextShrink : public RoseCommand
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

    void run(const RoseModel &s0, Rose<RoseModel::ValueT> &rose) const override
    {
        bool didShrink;
        auto value(rose.nextShrink(didShrink));

        RoseModel s1(nextState(s0));
        RC_ASSERT(value == s1.currentValue);
        RC_ASSERT(didShrink == s1.didShrink);
    }
};

struct AcceptShrink : public RoseCommand
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

    void run(const RoseModel &s0, Rose<RoseModel::ValueT> &rose) const override
    {
        rose.acceptShrink();
    }
};

class PassthroughGen : public gen::Generator<std::vector<int>>
{
public:
    PassthroughGen(const std::vector<int> &values) : m_values(values) {}

    std::vector<int> generate() const override
    {
        std::vector<int> values;
        for (auto value : m_values)
            values.push_back(*gen::constant(value));
        return values;
    }

    Seq<std::vector<int>> shrink(std::vector<int> value) const override
    { return seq::just(std::vector<int>{1, 2, 3}); }

private:
    std::vector<int> m_values;
};

} // namespace

TEST_CASE("Rose") {
    prop("stateful test",
         [] (const TestCase &testCase) {
             auto sizes =
                 *gen::collection<std::vector<std::size_t>>(gen::ranged(0, 10));

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
                 // TODO gotta be a better way to do this
                 ImplicitParam<param::Random> randomEngine(Random(testCase.seed));
                 ImplicitParam<param::Size> size(testCase.size);
                 ImplicitParam<param::CurrentNode> currentNode(nullptr);
                 s0.acceptedValue = generator.generate();
                 s0.currentValue = s0.acceptedValue;
                 s0.didShrink = false;
                 s0.i1 = 0;
                 s0.i2 = 0;
                 s0.se = 0;
             }

             Rose<RoseModel::ValueT> rose(&generator, testCase);
             state::check(s0, rose, state::anyCommand<
                          CurrentValue,
                          GetExample,
                          NextShrink,
                          AcceptShrink>);
         });

    prop("shrinking of one value does not affect other unrelated values",
         [] (const TestCase &testCase) {
             // TODO this test is a bit hard to understand, document or refactor
             auto size = *gen::ranged<std::size_t>(0, gen::currentSize() + 1);
             std::vector<ErraticSum> generators(size);
             auto generator = gen::scale(0.1, VectorGen<ErraticSum>(generators));
             Rose<std::vector<int>> rose(&generator, testCase);

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
             auto size = *gen::ranged<int>(0, gen::currentSize() + 1) + 1;
             auto i = *gen::ranged<int>(0, size);
             auto elementGen = gen::scale(0.05, gen::arbitrary<std::vector<uint8_t>>());
             OptionalShrink<decltype(elementGen)> shrinkGen(elementGen, true);
             OptionalShrink<decltype(elementGen)> noShrinkGen(elementGen, false);
             std::vector<decltype(shrinkGen)> generators(size, shrinkGen);
             generators[i] = noShrinkGen;
             VectorGen<decltype(shrinkGen)> generator(generators);

             Rose<gen::GeneratedT<decltype(generator)>> rose(&generator, testCase);
             auto original = rose.currentValue();
             bool didShrink = true;
             while (didShrink)
                 RC_ASSERT(rose.nextShrink(didShrink)[i] == original[i]);
         });

    SECTION("example") {
        prop("correctly generates examples when not shrunk",
             [] (const TestCase &testCase, std::vector<int> values)
             {
                 PassthroughGen generator(values);
                 Rose<std::vector<int>> rose(&generator, testCase);

                 auto example(rose.example());
                 RC_ASSERT(example.size() == values.size());

                 for (int i = 0; i < example.size(); i++) {
                     RC_ASSERT(example[i] ==
                               std::make_pair(
                                   typeToString<decltype(values)::value_type>(),
                                   toString(values[i])));
                 }
             });

        prop("generates empty example if shrunk",
             [] (const TestCase &testCase, std::vector<int> values)
             {
                 PassthroughGen generator(values);
                 Rose<std::vector<int>> rose(&generator, testCase);
                 bool didShrink;
                 rose.nextShrink(didShrink);
                 RC_ASSERT(didShrink);

                 auto example(rose.example());
                 RC_ASSERT(example.empty());
             });
    }
}
