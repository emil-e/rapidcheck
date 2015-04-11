#pragma once

#include <cassert>

namespace rc {
namespace state {
namespace detail {

//! Collection of commands.
template<typename Cmd>
class Commands
    : public Command<typename Cmd::State, typename Cmd::Sut>
{
public:
    typedef std::shared_ptr<const Cmd> CmdSP;
    typedef typename Cmd::State State;
    typedef typename Cmd::Sut Sut;

    //! Constructs a new `Commands` from the given command vector.
    explicit Commands(std::vector<CmdSP> commands =
                      std::vector<CmdSP>())
        : m_commands(std::move(commands)) {}

    //! Returns a reference to the vector of commands.
    const std::vector<CmdSP> &commands() const
    { return m_commands; }

    //! Appends a new command.
    void append(CmdSP command)
    { m_commands.push_back(std::move(command)); }

    State nextState(const State &state) const override
    {
        State currentState = state;
        for (const auto &command : m_commands)
            currentState = command->nextState(currentState);
        return currentState;
    }

    void run(const State &state, Sut &sut) const override
    {
        State currentState = state;
        for (const auto &command : m_commands) {
            auto nextState = command->nextState(currentState);
            command->run(currentState, sut);
            currentState = nextState;
        }
    }

    void show(std::ostream &os) const override
    {
        for (const auto &command : m_commands) {
            command->show(os);
            os << std::endl;
        }
    }

private:
    std::vector<CmdSP> m_commands;
};

//! Collection of commands.
template<typename Cmd>
struct NewCommands : public Command<typename Cmd::State, typename Cmd::Sut>
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

template<typename Cmd, typename GenerationFunc>
class CommandsGenerator : public gen::Generator<Commands<Cmd>>
{
public:
    typedef std::shared_ptr<const Cmd> CmdSP;
    typedef Commands<Cmd> Cmds;
    typedef typename Cmd::State State;
    typedef typename Cmd::Sut Sut;

    explicit CommandsGenerator(State initialState,
                               GenerationFunc generationFunc)
        : m_initialState(std::move(initialState))
        , m_generationFunc(std::move(generationFunc)) {}

    Cmds generate() const override
    {
        Cmds commands;
        State currentState(m_initialState);
        int tries = 0;
        while (commands.commands().size() < gen::currentSize()) {
            auto command = *gen::lambda([=] {
                return m_generationFunc(currentState);
            });

            try {
                currentState = command->nextState(currentState);
                commands.append(std::move(command));
            } catch (const ::rc::detail::CaseResult &result) {
                if (result.type != ::rc::detail::CaseResult::Type::Discard)
                    throw;
                tries++;
                if (tries > 100) {
                    throw gen::GenerationFailure(
                        "Gave up trying to generate command sequence of length " +
                        std::to_string(gen::currentSize()));
                }
            }
        }

        return commands;
    }

    Seq<Cmds> shrink(Cmds commands) const override
    {
        return seq::filter(
            seq::map(shrink::removeChunks(commands.commands()),
                [](std::vector<CmdSP> &&x) {
                    return Cmds(std::move(x));
                }),
            [&](const Cmds &commands) {
                return isValidCommand(commands, m_initialState);
            });
    }

private:
    State m_initialState;
    GenerationFunc m_generationFunc;
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

} // namespace detail

template<typename State, typename Sut>
State Command<State, Sut>::nextState(const State &s0) const
{ return s0; }

template<typename State, typename Sut>
void Command<State, Sut>::run(const State &s0, Sut &sut) const {}

template<typename State, typename Sut>
void Command<State, Sut>::show(std::ostream &os) const
{ os << ::rc::detail::demangle(typeid(*this).name()); }

template<typename State, typename Sut, typename GenerationFunc>
void check(State initialState,
           Sut &sut,
           GenerationFunc generationFunc)
{
    auto commands = *detail::CommandsGenerator<
        Command<State, Sut>,
        GenerationFunc>(initialState, generationFunc);
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
std::shared_ptr<const typename Cmd::CommandType>
anyCommand(const typename Cmd::State &state)
{
    // TODO configurable
    for (int tries = 0; tries < 100; tries++) {
        try {
            return detail::CommandPicker<Cmd, Cmds...>::pick(
                state,
                *gen::noShrink(gen::ranged<int>(0, sizeof...(Cmds) + 1)));
        } catch (const ::rc::detail::CaseResult &result) {
            if (result.type != ::rc::detail::CaseResult::Type::Discard)
                throw;
        }
    }

    throw gen::GenerationFailure("Failed to generate a valid command");
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
