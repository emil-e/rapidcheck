#include <catch.hpp>
#include <rapidcheck-catch.h>

#include <typeinfo>
#include <typeindex>

#include "util/ShowTypeTestUtils.h"
#include "util/GenUtils.h"
#include "util/ShrinkableUtils.h"

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
typedef NewCommands<StringVecCmd> StringVecCmdsN;

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

struct NewPushBack : public StringVecCmd
{
    NewPushBack() : value(*newgen::arbitrary<std::string>()) {}
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
    { rc::show(value, os); }
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

struct ParamsCmd : StringVecCmd
{
    Random random;
    int size;
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

Gen<std::vector<StringVecCmdSP>> pushBackCommands()
{
    return newgen::container<std::vector<StringVecCmdSP>>(
        newgen::exec([]() -> StringVecCmdSP {
            return std::make_shared<PushBack>();
        }));
}

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

TEST_CASE("NewCommands") {
    SECTION("nextState") {
        newprop(
            "returns next state by applying the commands in sequence",
            [] (const StringVec &s0) {
                StringVecCmdsN cmds;
                cmds.commands = *pushBackCommands();

                StringVec expected(s0);
                for (const auto &cmd : cmds.commands) {
                    std::ostringstream os;
                    cmd->show(os);
                    expected.push_back(os.str());
                }

                RC_ASSERT(cmds.nextState(s0) == expected);
            });
    }

    SECTION("run") {
        newprop(
            "runs the commands in sequence",
            [] (const StringVec &s0) {
                StringVecCmdsN cmds;
                cmds.commands = *pushBackCommands();

                StringVec expected(s0);
                for (const auto &cmd : cmds.commands) {
                    std::ostringstream os;
                    cmd->show(os);
                    expected.push_back(os.str());
                }

                StringVec actual(s0);
                cmds.run(s0, actual);
                RC_ASSERT(actual == expected);
            });
    }

    SECTION("show") {
        newprop(
            "displays each command on a separate line",
            [] {
                StringVecCmdsN cmds;
                cmds.commands = *pushBackCommands();

                std::ostringstream expected;
                for (const auto &cmd : cmds.commands) {
                    cmd->show(expected);
                    expected << std::endl;
                }

                std::ostringstream actual;
                cmds.show(actual);
                RC_ASSERT(expected.str() == actual.str());
            });
    }
}

TEST_CASE("isValidCommand") {
    newprop(
        "returns true for valid commands",
        [] {
            RC_ASSERT(isValidCommand(PushBack(), StringVec()));
        });

    newprop(
        "returns false for invalid commands",
        [] {
            RC_ASSERT(!isValidCommand(PopBack(), StringVec()));
        });
}

Gen<StringVecCmdSP> captureParams(const StringVec &vec)
{
    return [](const Random &random, int size) {
        auto paramsCmd = std::make_shared<ParamsCmd>();
        paramsCmd->random = random;
        paramsCmd->size = size;
        return shrinkable::just(
            std::static_pointer_cast<const StringVecCmd>(paramsCmd));
    };
}

std::vector<GenParams> collectParams(const StringVecCmdsN &cmds)
{
    std::vector<GenParams> params;
    std::transform(
        begin(cmds.commands), end(cmds.commands),
        std::back_inserter(params),
        [](const StringVecCmdSP &cmd) {
            const auto paramsCmd =
                std::static_pointer_cast<const ParamsCmd>(cmd);
            GenParams params;
            params.random = paramsCmd->random;
            params.size = paramsCmd->size;
            return params;
        });
    return params;
}

std::set<Random> collectRandoms(const StringVecCmdsN &cmds)
{
    const auto params = collectParams(cmds);
    std::set<Random> randoms;
    std::transform(
        begin(params), end(params),
        std::inserter(randoms, randoms.end()),
        [](const GenParams &params) { return params.random; });
    return randoms;
}

typedef std::vector<int> IntVec;
typedef Command<IntVec, IntVec> IntVecCmd;
typedef std::shared_ptr<const IntVecCmd> IntVecCmdSP;
typedef NewCommands<IntVecCmd> IntVecCmds;

struct CountCmd : public IntVecCmd
{
    CountCmd(int x) : value(x) {}
    int value;

    IntVec nextState(const IntVec &s0) const override
    {
        RC_PRE(!s0.empty());
        RC_PRE(s0.back() == (value - 1));
        IntVec s1(s0);
        s1.push_back(value);
        return s1;
    }

    void run(const IntVec &s0, IntVec &sut) const override
    { sut.push_back(value); }

    void show(std::ostream &os) const
    { os << value; }
};

TEST_CASE("genCommands") {
    newprop(
        "command sequences are always valid",
        [](const GenParams &params, const StringVec &s0) {
            const auto gen = genCommands<StringVecCmd>(
                s0, &newAnyCommand<NewPushBack, PopBack>);
            onAnyPath(
                gen(params.random, params.size),
                [&](const Shrinkable<StringVecCmdsN> &value,
                   const Shrinkable<StringVecCmdsN> &shrink)
                {
                    RC_ASSERT(isValidCommand(value.value(), s0));
                });
        });

    newprop(
        "shrinks are shorter or equal length when compared to original",
        [](const GenParams &params, const StringVec &s0) {
            const auto gen = genCommands<StringVecCmd>(
                s0, &newAnyCommand<NewPushBack, PopBack>);
            onAnyPath(
                gen(params.random, params.size),
                [&](const Shrinkable<StringVecCmdsN> &value,
                    const Shrinkable<StringVecCmdsN> &shrink)
                {
                    RC_ASSERT(value.value().commands.size() <=
                              value.value().commands.size());
                });
        });

    // NOTE: This property is not strictly true for all commands since a shrink
    // of a command may be equivalent to the original but in our case, this
    // won't happen
    newprop(
        "shrinks are not equivalent to original",
        [](const GenParams &params, const StringVec &s0) {
            const auto gen = genCommands<StringVecCmd>(
                s0, &newAnyCommand<NewPushBack, PopBack>);
            onAnyPath(
                gen(params.random, params.size),
                [&](const Shrinkable<StringVecCmdsN> &value,
                    const Shrinkable<StringVecCmdsN> &shrink)
                {
                    const auto valueCmd = value.value();
                    const auto shrinkCmd = shrink.value();
                    const auto valueResult = valueCmd.nextState(s0);
                    const auto shrinkResult = shrinkCmd.nextState(s0);
                    RC_ASSERT(valueResult != shrinkResult);
                });
        });

    newprop(
        "passed random generators are unique",
        [](const GenParams &params) {
            const auto gen = genCommands<StringVecCmd>(
                StringVec(), &captureParams);
            const auto cmds = gen(params.random, params.size).value();
            const auto randoms = collectRandoms(cmds);
            RC_ASSERT(randoms.size() == cmds.commands.size());
        });


    newprop(
        "shrinks use a subset of the original random generators",
        [](const GenParams &params) {
            const auto gen = genCommands<StringVecCmd>(
                StringVec(), &captureParams);
            onAnyPath(
                gen(params.random, params.size),
                [&](const Shrinkable<StringVecCmdsN> &value,
                    const Shrinkable<StringVecCmdsN> &shrink)
                {
                    const auto valueRandoms = collectRandoms(value.value());
                    const auto shrinkRandoms = collectRandoms(shrink.value());
                    std::vector<Random> intersection;
                    std::set_intersection(begin(valueRandoms), end(valueRandoms),
                                          begin(shrinkRandoms), end(shrinkRandoms),
                                          std::back_inserter(intersection));
                    RC_ASSERT(intersection.size() == shrinkRandoms.size());
                });
        });

    newprop(
        "passes the correct size",
        [](const GenParams &params) {
            const auto gen = genCommands<StringVecCmd>(
                StringVec(), &captureParams);
            onAnyPath(
                gen(params.random, params.size),
                [&](const Shrinkable<StringVecCmdsN> &value,
                    const Shrinkable<StringVecCmdsN> &shrink)
                {
                    const auto allParams = collectParams(value.value());
                    RC_ASSERT(std::all_of(
                                  begin(allParams), end(allParams),
                                  [&](const GenParams &p) {
                                      return p.size == params.size;
                                  }));
                });
        });

    newprop(
        "correctly threads the state when generating commands",
        [](const GenParams &params) {
            IntVec s0({0});
            const auto gen = genCommands<IntVecCmd>(
                s0, [](const IntVec &vec) {
                    auto cmd = std::make_shared<const CountCmd>(vec.back() + 1);
                    return newgen::just(
                        std::move(std::static_pointer_cast<const IntVecCmd>(cmd)));
                });

            onAnyPath(
                gen(params.random, params.size),
                [&](const Shrinkable<IntVecCmds> &value,
                   const Shrinkable<IntVecCmds> &shrink) {
                    auto sut = s0;
                    value.value().run(s0, sut);
                    int x = 0;
                    for (int value : sut)
                        RC_ASSERT(value == x++);
                });
        });

    newprop(
        "finds minimum where one commands always fails",
        [](const GenParams &params, const StringVec &s0) {
            const auto gen = genCommands<StringVecCmd>(
                s0, &newAnyCommand<
                AlwaysFail,
                NewPushBack,
                PopBack,
                SomeCommand>);
            const auto result = searchGen(
                params.random, params.size, gen,
                [&](const StringVecCmdsN &cmds) {
                    try {
                        StringVec sut = s0;
                        cmds.run(s0, sut);
                    } catch (...) {
                        return true;
                    }
                    return false;

                });

            RC_ASSERT(result.commands.size() == 1);
            std::ostringstream os;
            result.commands.front()->show(os);
            RC_ASSERT(os.str().find("AlwaysFail") != std::string::npos);
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

TEST_CASE("newAnyCommand") {
    newprop(
        "returns one of the commands",
        [] (const GenParams &params, const StringVec &s0) {
            const auto cmd = newAnyCommand<A, B, C>(s0)(
                params.random, params.size).value();
            const auto &id = typeid(*cmd);
            RC_ASSERT((id == typeid(A)) ||
                      (id == typeid(B)) ||
                      (id == typeid(C)));
        });

    newprop(
        "all commands are eventually returned",
        [] (const GenParams &params, const StringVec &s0) {
            auto r = params.random;
            const auto gen = newAnyCommand<A, B, C>(s0);
            std::set<std::type_index> all{ typeid(A), typeid(B), typeid(C) };
            std::set<std::type_index> generated;
            while (generated != all)
                generated.emplace(typeid(*gen(r.split(), params.size).value()));
            RC_SUCCEED("All generated");
        });

    newprop(
        "uses state constructor if there is one, passing it the state",
        [] (const GenParams &params, const StringVec &s0) {
            const auto cmd = newAnyCommand<DualConstructible>(s0)(
                params.random, params.size).value();
            RC_ASSERT(static_cast<const DualConstructible &>(*cmd).state ==
                      s0);
        });
}

TEST_CASE("show(Command)") {
    newprop(
        "passing a generic command to show yields the same as Command::show",
        [] {
            PushBack cmd;
            std::ostringstream expected;
            cmd.show(expected);
            std::ostringstream actual;
            show(cmd, actual);
            RC_ASSERT(actual.str() == expected.str());
        });

    newprop(
        "passing Commands to show yields the same result as Commands::show",
        [] {
            StringVecCmds commands(*pushBackCommands());
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

TEST_CASE("state::newcheck") {
    newprop(
        "if no command fails, check succeeds",
        [](const StringVec &s0, StringVec sut) {
            state::newcheck(s0, sut, &newAnyCommand<PushBack>);
        });

    newprop(
        "if some command fails, check fails",
        [](const StringVec &s0, StringVec sut) {
            try {
                state::newcheck(s0, sut, &newAnyCommand<AlwaysFail>);
                RC_FAIL("Check succeeded");
            } catch (const CaseResult &result) {
                RC_ASSERT(result.type == CaseResult::Type::Failure);
            }
        });
}
