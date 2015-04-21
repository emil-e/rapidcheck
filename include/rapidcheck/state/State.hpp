#pragma once

#include <algorithm>
#include <cassert>

namespace rc {
namespace state {
namespace detail {

//! Collection of commands.
template<typename Cmd>
struct Commands : public Command<typename Cmd::State, typename Cmd::Sut>
{
public:
    typedef std::shared_ptr<const Cmd> CmdSP;
    typedef typename Cmd::State State;
    typedef typename Cmd::Sut Sut;

    State nextState(const State &state) const override
    {
        State currentState = state;
        for (const auto &command : commands)
            currentState = command->nextState(currentState);
        return currentState;
    }

    void run(const State &state, Sut &sut) const override
    {
        State currentState = state;
        for (const auto &command : commands) {
            auto nextState = command->nextState(currentState);
            command->run(currentState, sut);
            currentState = nextState;
        }
    }

    void show(std::ostream &os) const override
    {
        for (const auto &command : commands) {
            command->show(os);
            os << std::endl;
        }
    }

    std::vector<CmdSP> commands;
};

template<typename Cmd, typename GenFunc>
class CommandsGen
{
public:
    typedef std::shared_ptr<const Cmd> CmdSP;
    typedef typename Cmd::State State;
    typedef typename Cmd::Sut Sut;

    template<typename StateArg, typename GenFuncArg>
    CommandsGen(StateArg &&initialState, GenFuncArg &&genFunc)
        : m_initialState(std::forward<StateArg>(initialState))
        , m_genFunc(std::forward<GenFuncArg>(genFunc)) {}

    Shrinkable<Commands<Cmd>> operator()(const Random &random,
                                         int size) const
    {
        return generateCommands(random, size);
    }

private:
    struct CommandEntry
    {
        CommandEntry(Random &&aRandom,
                     Shrinkable<CmdSP> &&aShrinkable,
                     State &&aState)
            : random(std::move(aRandom))
            , shrinkable(std::move(aShrinkable))
            , postState(std::move(aState)) {}

        Random random;
        Shrinkable<CmdSP> shrinkable;
        State postState;
    };

    struct CommandSequence
    {
        CommandSequence(const State &initState, const GenFunc &func, int sz)
            : initialState(initState)
            , genFunc(func)
            , size(sz)
            , numFixed(0) {}

        State initialState;
        GenFunc genFunc;
        int size;
        std::size_t numFixed;
        std::vector<CommandEntry> entries;

        const State &stateAt(std::size_t i) const
        {
            if (i <= 0)
                return initialState;
            return entries[i - 1].postState;
        }

        void repairEntriesFrom(std::size_t start)
        {
            for (auto i = start; i < entries.size(); i++) {
                if (!repairEntryAt(i))
                    entries.erase(begin(entries) + i--);
            }
        }

        bool repairEntryAt(std::size_t i)
        {
            using namespace ::rc::detail;
            try  {
                auto &entry = entries[i];
                const auto cmd = entry.shrinkable.value();
                entry.postState = cmd->nextState(stateAt(i));
            } catch (const CaseResult &result) {
                if (result.type != CaseResult::Type::Discard)
                    throw;

                return regenerateEntryAt(i);
            }

            return true;
        }

        bool regenerateEntryAt(std::size_t i)
        {
            using namespace ::rc::detail;
            try {
                auto &entry = entries[i];
                const auto &preState = stateAt(i);
                entry.shrinkable = genFunc(preState)(entry.random, size);
                const auto cmd = entry.shrinkable.value();
                entry.postState = cmd->nextState(preState);
            } catch (const CaseResult &result) {
                if (result.type != CaseResult::Type::Discard)
                    throw;

                return false;
            }

            return true;
        }
    };

    Shrinkable<Commands<Cmd>> generateCommands(const Random &random,
                                               int size) const
    {
        return shrinkable::map(
            generateSequence(random, size),
            [](const CommandSequence &sequence) {
                Commands<Cmd> cmds;
                const auto &entries = sequence.entries;
                cmds.commands.reserve(entries.size());
                std::transform(
                    begin(entries), end(entries),
                    std::back_inserter(cmds.commands),
                    [](const CommandEntry &entry) {
                        return entry.shrinkable.value();
                    });
                return cmds;
            });
    }

    Shrinkable<CommandSequence> generateSequence(const Random &random,
                                                 int size) const
    {
        Random r(random);
        std::size_t count = (r.split().next() % (size + 1)) + 1;
        return shrinkable::shrinkRecur(
            generateInitial(random, size, count),
            &shrinkSequence);
    }

    CommandSequence generateInitial(const Random &random,
                                    int size,
                                    std::size_t count) const
    {
        CommandSequence sequence(m_initialState, m_genFunc, size);
        sequence.entries.reserve(count);

        auto *state = &m_initialState;
        auto r = random;
        while (sequence.entries.size() < count) {
            sequence.entries.push_back(nextEntry(r.split(), size, *state));
            state = &sequence.entries.back().postState;
        }

        return sequence;
    }

    CommandEntry nextEntry(const Random &random,
                           int size,
                           const State &state) const
    {
        using namespace ::rc::detail;
        auto r = random;
        // TODO configurability?
        for (int tries = 0; tries < 100; tries++) {
            try {
                auto random = r.split();
                auto shrinkable = m_genFunc(state)(random, size);
                auto postState = shrinkable.value()->nextState(state);

                return CommandEntry(std::move(random),
                                    std::move(shrinkable),
                                    std::move(postState));
            } catch (const CaseResult &result) {
                if (result.type != CaseResult::Type::Discard)
                    throw;
                // What to do?
            } catch (const GenerationFailure &failure) {
                // What to do?
            }
        }

        // TODO better error message
        throw GenerationFailure("Failed to generate command after 100 tries.");
    }

    static Seq<CommandSequence> shrinkSequence(const CommandSequence &s)
    { return seq::concat(shrinkRemoving(s), shrinkIndividual(s)); }

    static Seq<CommandSequence> shrinkRemoving(const CommandSequence &s)
    {
        auto nonEmptyRanges = seq::subranges(s.numFixed, s.entries.size());
        return seq::map(
            std::move(nonEmptyRanges),
            [=](const std::pair<std::size_t, std::size_t> &r) {
                auto shrunk = s;
                shrunk.entries.erase(
                    begin(shrunk.entries) + r.first,
                    begin(shrunk.entries) + r.second);
                shrunk.repairEntriesFrom(r.first);
                return shrunk;
            });
    }

    static Seq<CommandSequence> shrinkIndividual(const CommandSequence &s)
    {
        return seq::mapcat(
            seq::range(s.numFixed, s.entries.size()),
            [=](std::size_t i) {
                const auto &preState = s.stateAt(i);
                auto valid = seq::filter(
                    s.entries[i].shrinkable.shrinks(),
                    [=](const Shrinkable<CmdSP> &s) {
                        return isValidCommand(*s.value(), preState);
                    });

                return seq::map(
                    std::move(valid),
                    [=](Shrinkable<CmdSP> &&cmd) {
                        auto shrunk = s;
                        shrunk.entries[i].shrinkable = std::move(cmd);
                        shrunk.numFixed = i;
                        shrunk.repairEntriesFrom(i);
                        return shrunk;
                    });
            });
    }

    State m_initialState;
    GenFunc m_genFunc;
};

template<typename Cmd, typename = typename std::is_constructible<
                           Cmd, typename Cmd::State &&>::type>
    struct CommandMaker;

template<typename Cmd>
struct CommandMaker<Cmd, std::true_type>
{
    static std::shared_ptr<const typename Cmd::CommandType> make(
        const typename Cmd::State &state)
    { return std::make_shared<Cmd>(state); }
};

template<typename Cmd>
struct CommandMaker<Cmd, std::false_type>
{
    static std::shared_ptr<const typename Cmd::CommandType> make(
        const typename Cmd::State &state)
    { return std::make_shared<Cmd>(); }
};

template<typename ...Cmds>
struct CommandPicker;

template<typename Cmd>
struct CommandPicker<Cmd>
{
    static std::shared_ptr<const typename Cmd::CommandType> pick(
        const typename Cmd::State &state, int n)
    {
        return CommandMaker<Cmd>::make(state);
    }
};

template<typename Cmd, typename ...Cmds>
struct CommandPicker<Cmd, Cmds...>
{
    static std::shared_ptr<const typename Cmd::CommandType> pick(
        const typename Cmd::State &state, int n)
    {
        return (n == 0)
            ? CommandMaker<Cmd>::make(state)
            : CommandPicker<Cmds...>::pick(state, n - 1);
    }
};

template<typename Cmd, typename State, typename GenerationFunc>
Gen<Commands<Cmd>> genCommands(State &&initialState, GenerationFunc &&genFunc)
{
    return detail::CommandsGen<Cmd, Decay<GenerationFunc>>(
        std::forward<State>(initialState),
        std::forward<GenerationFunc>(genFunc));
}

} // namespace detail

template<typename State, typename Sut>
State Command<State, Sut>::nextState(const State &s0) const
{ return s0; }

template<typename State, typename Sut>
void Command<State, Sut>::run(const State &s0, Sut &sut) const {}

template<typename State, typename Sut>
void Command<State, Sut>::show(std::ostream &os) const
{ os << ::rc::detail::demangle(typeid(*this).name()); }

template<typename State, typename Sut, typename GenFunc>
void check(const State &initialState,
           Sut &sut,
           GenFunc &&generationFunc)
{
    const auto commands = *detail::genCommands<Command<Decay<State>, Sut>>(
        initialState, std::forward<GenFunc>(generationFunc));
    commands.run(initialState, sut);
}

template<typename State, typename Sut>
bool isValidCommand(const Command<State, Sut> &command, const State &s0)
{
    try {
        command.nextState(s0);
    } catch (const ::rc::detail::CaseResult &result) {
        if (result.type == ::rc::detail::CaseResult::Type::Discard)
            return false;
        throw;
    }

    return true;
}

template<typename Cmd, typename ...Cmds>
Gen<std::shared_ptr<const typename Cmd::CommandType>> anyCommand(
    const typename Cmd::State &state)
{
    return [=](const Random &random, int size) {
        auto r = random;
        std::size_t n = r.split().next() % (sizeof...(Cmds) + 1);
        return gen::exec([=]{
            return detail::CommandPicker<Cmd, Cmds...>::pick(state, n);
        })(r, size); // TODO monadic bind
    };
}

template<typename State, typename Sut>
void showValue(const Command<State, Sut> &command, std::ostream &os)
{ command.show(os); }

} // namespace state

template<typename State, typename Sut>
struct ShowType<rc::state::Command<State, Sut>>
{
    static void showType(std::ostream &os)
    {
        os << "Command<";
        ::rc::detail::showType<State>(os);
        os << ", ";
        ::rc::detail::showType<Sut>(os);
        os << ">";
    }
};

template<typename Cmd>
struct ShowType<rc::state::detail::Commands<Cmd>>
{
    static void showType(std::ostream &os)
    {
        os << "Commands<";
        ::rc::detail::showType<Cmd>(os);
        os << ">";
    }
};

} // namespace rc
