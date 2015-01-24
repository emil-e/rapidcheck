#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <rapidcheck-catch.h>

using namespace rc;

class Counter
{
public:
    void inc()
    {
        m_value++;
    }

    void dec() {
        assert(m_value > 0);
        // Broken!
        if (m_value != 10)
            m_value--;
    }

    int get() { return m_value; }

private:
    int m_value = 0;
};

struct CounterModel
{
    int value = 0;
};

struct Inc : public state::Command<CounterModel, Counter>
{
    CounterModel nextState(const CounterModel &s0) const override
    {
        CounterModel s1(s0);
        s1.value++;
        return s1;
    }

    void run(const CounterModel &s0, Counter &counter) const override
    {
        auto prev = counter.get();
        counter.inc();
        RC_ASSERT(counter.get() == (s0.value + 1));
    }
};

struct Dec : public state::Command<CounterModel, Counter>
{
    CounterModel nextState(const CounterModel &state) const override
    {
        RC_PRE(state.value > 0);
        CounterModel s1(state);
        s1.value--;
        return s1;
    }

    void run(const CounterModel &state, Counter &counter) const override
    {
        auto prev = counter.get();
        counter.dec();
        RC_ASSERT(counter.get() == (state.value - 1));
    }
};

TEST_CASE("Counter") {
    prop("state test",
         [] {
             CounterModel state;
             Counter sut;
             state::check(state, sut, [] (const CounterModel &state) {
                 if (*gen::arbitrary<bool>())
                     return state::CommandSP<CounterModel, Counter>(new Inc());
                 else
                     return state::CommandSP<CounterModel, Counter>(new Dec());
             });
         });
}
