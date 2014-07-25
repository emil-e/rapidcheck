#pragma once

namespace rc {
namespace state {

template<typename State, typename Sut>
State Command<State, Sut>::nextState(const State &s0) const
{ return s0; }

template<typename State, typename Sut>
void Command<State, Sut>::run(const State &s0, Sut &sut) const {}

template<typename State, typename Sut>
void Command<State, Sut>::show(std::ostream &os) const
{ os << detail::demangle(typeid(*this).name()); }

template<typename State, typename Sut>
void show(const Command<State, Sut> &command, std::ostream &os)
{ command.show(os); }

//! Collection of commands.
template<typename CommandT>
class Commands
    : public Command<typename CommandT::StateT, typename CommandT::SutT>
{
public:
    typedef typename CommandT::StateT StateT;
    typedef typename CommandT::SutT SutT;

    //! Constructs a new `Commands` from the given command vector.
    explicit Commands(std::vector<CommandSP<StateT, SutT>> commands =
                      std::vector<CommandSP<StateT, SutT>>())
        : m_commands(std::move(commands)) {}

    //! Returns a reference to the vector of commands.
    const std::vector<CommandSP<StateT, SutT>> &commands() const
    { return m_commands; }

    //! Appends a new command.
    void append(CommandSP<StateT, SutT> command)
    { m_commands.push_back(command); }

    StateT nextState(const StateT &state) const override
    {
        StateT currentState = state;
        for (const auto &command : m_commands)
            currentState = command->nextState(currentState);
        return currentState;
    }

    void run(const StateT &state, SutT &sut) const override
    {
        StateT currentState = state;
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
    std::vector<CommandSP<StateT, SutT>> m_commands;
};

template<typename GenerationFunc, typename Command>
class CommandsGenerator : public gen::Generator<Commands<Command>>
{
public:
    typedef Command CommandT;
    typedef Commands<CommandT> CommandsT;
    typedef typename CommandT::StateT StateT;
    typedef typename CommandT::SutT SutT;

    explicit CommandsGenerator(StateT initialState,
                               GenerationFunc generationFunc)
        : m_initialState(std::move(initialState))
        , m_generationFunc(std::move(generationFunc)) {}

    CommandsT generate() const override
    {
        CommandsT commands;
        StateT currentState(m_initialState);
        while (commands.commands().size() < gen::currentSize()) {
            auto command = pick(gen::lambda([=] {
                return m_generationFunc(currentState);
            }));

            try {
                currentState = command->nextState(currentState);
                commands.append(std::move(command));
            } catch (const detail::CaseResult &result) {
                if (result.type() != detail::CaseResult::Type::Discard)
                    throw;
            }
        }

        return commands;
    }

    shrink::IteratorUP<CommandsT> shrink(CommandsT commands) const override
    {
        return shrink::map(
            shrink::removeChunks(commands.commands()),
            [] (std::vector<CommandSP<StateT, SutT>> &&x) {
                return CommandsT(std::move(x));
            });
    }

private:
    StateT m_initialState;
    GenerationFunc m_generationFunc;
};

template<typename State, typename Sut, typename GenerationFunc>
void check(State initialState,
           Sut &sut,
           GenerationFunc generationFunc)
{
    auto commands = pick(CommandsGenerator<
                         GenerationFunc,
                         Command<State, Sut>>(initialState, generationFunc));
    commands.run(initialState, sut);
}

} // namespace state

template<typename CommandT>
struct Show<state::Commands<CommandT>>
{
    static void show(const state::Commands<CommandT> &commands, std::ostream &os)
    { commands.show(os); }
};

} // namespace rc
