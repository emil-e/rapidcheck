#include <catch.hpp>
#include <rapidcheck-catch.h>

#include <typeinfo>
#include <typeindex>

#include "util/ShowTypeTestUtils.h"

#include "rapidcheck/seq/Operations.h"

using namespace rc;
using namespace rc::test;
using namespace rc::detail;
using namespace rc::state;
using namespace rc::state::detail;

namespace {

typedef std::vector<std::string> StringVec;
typedef Command<StringVec, StringVec> StringVecCmd;
typedef std::shared_ptr<const StringVecCmd> StringVecCmdSP;
typedef Commands<StringVecCmd> StringVecCmds;

struct PushBack : public StringVecCmd
{
    PushBack() : value(*gen::arbitrary<std::string>()) {}
    std::string value;

    StringVec nextState(const StringVec &s0) const override
    {
        StringVec s1(s0);
        s1.push_back(value);
        return s1;
    }

    void run(const StringVec &s0, StringVec &sut) const override
    { sut.push_back(value); }

    void show(std::ostream &os) const
    { os << value; }
};

struct PopBack : public StringVecCmd
{
    StringVec nextState(const StringVec &s0) const override
    {
        RC_PRE(!s0.empty());
        StringVec s1(s0);
        s1.pop_back();
        return s1;
    }

    void run(const StringVec &s0, StringVec &sut) const override
    { sut.pop_back(); }
};

struct AlwaysFail : public StringVecCmd
{
    void run(const StringVec &s0, StringVec &sut) const override
    { RC_FAIL("Always fails"); }
};

struct PreNeverHolds : public StringVecCmd
{
    StringVec nextState(const StringVec &s0) const override
    { RC_DISCARD("Preconditions never hold"); }
};

struct SomeCommand : StringVecCmd {};

class PushBackCommands : public gen::Generator<std::vector<StringVecCmdSP>>
{
public:
    std::vector<StringVecCmdSP> generate() const override
    {
        return *gen::collection<std::vector<StringVecCmdSP>>(
            gen::lambda([]() -> StringVecCmdSP {
                    return std::make_shared<PushBack>();
                }));
    }
};

struct A : public StringVecCmd {};
struct B : public StringVecCmd {};
struct C : public StringVecCmd {};

struct DualConstructible : public StringVecCmd
{
    DualConstructible();
    DualConstructible(const StringVec &s) : state(s) {}
    StringVec state;
};

struct AlwaysDiscard : public StringVecCmd
{
    AlwaysDiscard() { RC_DISCARD("Nope"); }
};

template<typename CommandT, typename GenerationFunc>
CommandsGenerator<CommandT, GenerationFunc> commandsGenerator(
    const typename CommandT::State &s0, GenerationFunc func)
{
    return CommandsGenerator<CommandT, GenerationFunc>(s0, std::move(func));
}

} // namespace

TEST_CASE("Command") {
    SECTION("nextState") {
        prop("default implementation returns unmodified state",
             [] (const StringVec &state) {
                 RC_ASSERT(StringVecCmd().nextState(state) == state);
             });
    }

    SECTION("run") {
        prop("default implementation does not modify SUT",
             [] (const StringVec &state, const StringVec &sut) {
                 const auto pre = sut;
                 auto post = sut;
                 StringVecCmd().run(state, post);
                 RC_ASSERT(pre == post);
             });
    }

    SECTION("show") {
        SECTION("outputs a string which contains the name of the class") {
            // TODO this is the only restriction I dare to place on this right
            // now until we can maybe get some sanitizing going
            std::ostringstream os;
            SomeCommand().show(os);
            REQUIRE(os.str().find("SomeCommand") != std::string::npos);
        }
    }
}

TEST_CASE("Commands") {
    SECTION("commands") {
        prop("returns the contained",
             [] {
                 const auto cmds = *PushBackCommands();
                 StringVecCmds commands(cmds);
                 RC_ASSERT(commands.commands() == cmds);
             });
    }

    SECTION("append") {
        prop("appends the given command",
             [] {
                 const auto cmd = std::make_shared<PushBack>();
                 auto cmds = *PushBackCommands();
                 StringVecCmds commands(cmds);
                 commands.append(cmd);
                 cmds.push_back(cmd);
                 RC_ASSERT(commands.commands() == cmds);
             });
    }

    SECTION("nextState") {
        prop("returns next state by applying the commands in sequence",
             [] (const StringVec &s0) {
                 const auto cmds = *PushBackCommands();
                 StringVecCmds commands(cmds);

                 StringVec expected(s0);
                 for (const auto &cmd : cmds) {
                     std::ostringstream os;
                     cmd->show(os);
                     expected.push_back(os.str());
                 }

                 RC_ASSERT(commands.nextState(s0) == expected);
             });
    }

    SECTION("run") {
        prop("runs the commands in sequence",
             [] (const StringVec &s0) {
                 const auto cmds = *PushBackCommands();
                 StringVecCmds commands(cmds);

                 StringVec expected(s0);
                 for (const auto &cmd : cmds) {
                     std::ostringstream os;
                     cmd->show(os);
                     expected.push_back(os.str());
                 }

                 StringVec actual(s0);
                 commands.run(s0, actual);
                 RC_ASSERT(actual == expected);
             });
    }

    SECTION("show") {
        prop("displays each command on a separate line",
             [] {
                 const auto cmds = *PushBackCommands();
                 StringVecCmds commands(cmds);

                 std::ostringstream expected;
                 for (const auto &cmd : cmds) {
                     cmd->show(expected);
                     expected << std::endl;
                 }

                 std::ostringstream actual;
                 commands.show(actual);
                 RC_ASSERT(expected.str() == actual.str());
             });
    }
}

TEST_CASE("isValidCommand") {
    prop("returns true for valid commands",
         [] {
             RC_ASSERT(isValidCommand(PushBack(), StringVec()));
         });

    prop("returns false for invalid commands",
         [] {
             RC_ASSERT(!isValidCommand(PopBack(), StringVec()));
         });
}

TEST_CASE("CommandsGenerator") {
    prop("command sequences are always valid",
         [] (const StringVec &s0) {
             const auto commands = *commandsGenerator<StringVecCmd>(
                 s0, &anyCommand<PushBack, PopBack>);
             RC_ASSERT(isValidCommand(commands, s0));
         });

    prop("number of generated commands is equal to current size",
         [] (const StringVec &s0) {
             const auto commands = *commandsGenerator<StringVecCmd>(
                 s0, anyCommand<PushBack, PopBack>);
             RC_ASSERT(commands.commands().size() == gen::currentSize());
         });

    prop("passes in the current state to the generation function",
         [] (const StringVec &s0) {
             StringVec currentState(s0);
             const auto commands = *commandsGenerator<StringVecCmd>(
                 s0, [&] (const StringVec &s0) {
                     RC_ASSERT(s0 == currentState);
                     const auto cmd = anyCommand<PushBack, PopBack>(s0);
                     if (isValidCommand(*cmd, s0))
                         currentState = cmd->nextState(s0);
                     return cmd;
                 });
         });

    prop("eventually gives up (GenerationFailure) if more commands cannot be"
         " generated",
         [] (const StringVec &s0) {
             int size = gen::currentSize() + 1;
             int targetSuccess = *gen::ranged(0, size);
             int numSuccess = 0;
             try {
                 const auto commands = *gen::resize(
                     size,
                     commandsGenerator<StringVecCmd>(
                         s0, [&] (const StringVec &s0) -> StringVecCmdSP {
                             if (numSuccess < targetSuccess) {
                                 numSuccess++;
                                 return std::make_shared<PushBack>();
                             } else {
                                 return std::make_shared<PreNeverHolds>();
                             }
                         }));
                 RC_FAIL("No exception was thrown");
             } catch (const gen::GenerationFailure &) {
                 RC_SUCCEED("GenerationFailure was thrown");
             } catch (...) {
                 RC_FAIL("Some other exception type was thrown");
             }
         });

    prop("shrinking yields only valid commands",
         [] (const StringVec &s0) {
             const auto generator =
                 gen::scale(0.25, commandsGenerator<StringVecCmd>(
                                s0, anyCommand<PushBack, PopBack>));
             const auto commands = *generator;
             auto seq = generator.shrink(commands);
             seq::forEach(std::move(seq), [&](const StringVecCmds &cmd) {
                 RC_ASSERT(isValidCommand(cmd, s0));
             });
         });
}

TEST_CASE("anyCommand") {
    prop("returns one of the commands",
         [] (const StringVec &s0) {
             const auto cmd = anyCommand<A, B, C>(s0);
             const auto &id = typeid(*cmd);
             RC_ASSERT((id == typeid(A)) ||
                       (id == typeid(B)) ||
                       (id == typeid(C)));
         });

    prop("all commands are eventually returned",
         [] (const StringVec &s0) {
             std::set<std::type_index> all{ typeid(A), typeid(B), typeid(C) };
             std::set<std::type_index> generated;
             while (generated != all)
                 generated.emplace(typeid(*anyCommand<A, B, C>(s0)));
             RC_SUCCEED("All generated");
         });

    prop("uses state constructor if there is one, passing it the state",
         [] (const StringVec &s0) {
             const auto cmd = anyCommand<DualConstructible>(s0);
             RC_ASSERT(static_cast<const DualConstructible &>(*cmd).state ==
                       s0);
         });

    prop("if a command constructor throws a discard result, it will not be"
         " chosen",
         [] (const StringVec &s0) {
             const auto cmd = anyCommand<A, AlwaysDiscard>(s0);
             RC_ASSERT(typeid(*cmd) != typeid(AlwaysDiscard));
         });

    prop("eventually gives up (GenerationFailure) if no valid command can be"
         " generated",
         [] (const StringVec &s0) {
             try {
                 const auto cmd = anyCommand<AlwaysDiscard>(s0);
                 RC_FAIL("No exception thrown");
             } catch (const gen::GenerationFailure &) {
                 RC_SUCCEED("GenerationFailure thrown");
             } catch (...) {
                 RC_FAIL("Other exception thrown");
             }
         });
}

TEST_CASE("show(Command)") {
    prop("passing a generic command to show yields the same as Command::show",
         [] {
             PushBack cmd;
             std::ostringstream expected;
             cmd.show(expected);
             std::ostringstream actual;
             show(cmd, actual);
             RC_ASSERT(actual.str() == expected.str());
         });

    prop("passing Commands to show yields the same result as Commands::show",
         [] {
             StringVecCmds commands(*PushBackCommands());
             std::ostringstream expected;
             commands.show(expected);
             std::ostringstream actual;
             show(commands, actual);
             RC_ASSERT(actual.str() == expected.str());
         });
}

TEST_CASE("typeToString<Command<...>>") {
    typedef Command<Foo, Bar> CommandT;
    REQUIRE(typeToString<CommandT>() == "Command<FFoo, BBar>");
}

TEST_CASE("typeToString<Commands<...>>") {
    typedef Commands<Foo> CommandsT;
    REQUIRE(typeToString<CommandsT>() == "Commands<FFoo>");
}

TEST_CASE("state::check") {
    prop("if no command fails, check succeeds",
         [](const StringVec &s0, StringVec sut) {
             state::check(s0, sut, anyCommand<PushBack>);
         });

    prop("if some command fails, check fails",
         [](const StringVec &s0, StringVec sut) {
             try {
                 state::check(s0, sut, anyCommand<AlwaysFail>);
                 RC_FAIL("Check succeeded");
             } catch (const CaseResult &result) {
                 RC_ASSERT(result.type == CaseResult::Type::Failure);
             }
         });
}
