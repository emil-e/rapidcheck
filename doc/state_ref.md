# Stateful testing - reference #
All commands that are part of the state testing framework reside under the `rc::state` namespace. To use them, you must include the `rapidcheck/state.h` header. Type signatures are not to be taken literally.

## Testing ##
#### `void check(Model initialState, Sut sut, GenFunc f)`<br/>`void check(MakeModel makeInitialState, Sut sut, GenFunc f)` ####
Generates a valid command sequence for an initial state and then runs that sequence on `sut`. Any assertions triggered when running the command sequence will cause the property to fail. For more information about the generation function parameter, see `rc::state::gen::commands`.

There are two overloads, one that takes the initial state as an immediate value and and another that takes a callable that returns the initial state.

This function must be used inside a property, it cannot be used standalone.

## `Command<Model, Sut>` ##
Represents an operation in the state testing framework. The `Model` type parameter is the type of the model that models `Sut` which is the actual System Under Test. These can also be accessed through the `Model` and `Sut` member type alises.

#### `virtual void checkPreconditions(const Model &s0) const` ####
If your command is not valid for all states, you must implement this method to assert preconditions. Preconditions can be asserted using any of the discarding macros such as `RC_PRE` or `RC_DISCARD`. If the command is discarded, RapidCheck will simply try to generate a new one. This method is intended to be overriden but has a default implementation that does nothing which is what you want if your command has no preconditions.

While the model state is passed as `const`, this doesn't prevent modification of the model if it, for example, is a `shared_ptr` or similar. Regardless, modifying the model state in this method leads to undefined behavior.

#### `virtual void apply(Model &s0) const` ####
Applies this command to the given state. The effect of this command on the model should be equivalent to this commands effect on the System Under Test. This method is intended to be overriden but has a default implementation that does nothing, something that can be useful for commands that do not modify state.

#### `virtual void run(const Model &s0, Sut &sut) const` ####
Applies this command to the given System Under Test. The state before this command has been applied is also passed in. If you need the post state, you can get this using the `nextState` convenience method. This is the method in which to place your assertions (`RC_ASSERT` et al.). This method is intended to be overriden but has a default implementation that does nothing.

While the model state is passed as `const`, this doesn't prevent modification of the model if it, for example, is a `shared_ptr` or similar. Regardless, modifying the model state in this method leads to undefined behavior.

#### `virtual void show(std::ostream &os) const` ####
Outputs a string representation of the command to the given output stream. The default implementation outputs the type name (via RTTI) but if your command has any sort of parameters, you will likely want to override this with a custom implementation to include those.

#### `Model nextState(const Model &s0) const` ####
Convenience method that calls `apply` on a copy of the given state and returns it. Saves some keystrokes occasionally since you don't have to allocate a model on the stack. Cannot be overridden, override `apply` instead.

This method does not work for non-copyable models.

## Command sequences ##
#### `Commands<CommandType>` ####
Type alias for a `std::vector<std::shared_ptr<const CommandType>>`. Useful since command sequences are generally stored in vectors and individual commands are usually passed by `shared_ptr`-to-const.

#### `void applyAll(Commands commands, Model &state)` ####
Applies the given commands in sequence to the given model state.

#### `void runAll(Commands commands, Model state, Sut &sut)`<br/>`void runAll(Commands commands, MakeModel makeState, Sut &sut)` ####
Runs the given commands in sequence on the given System Under Test assuming it is equivalent to the given model state.

There are two overloads, one that takes the state as an immediate value and and another that takes a callable that returns the state.

#### `bool isValidSequence(Commands commands, Model state)`<br/>`bool isValidSequence(Commands commands, MakeModel makeState)` ####
Returns `true` if the given command sequence is valid for the given model state, otherwise `false`.

There are two overloads, one that takes the state as an immediate value and and another that takes a callable that returns the state.

## Generators ##
#### `MakeCommandGenerator gen::execOneOfWithArgs<Ts...>()` ####
Returns a callable that when called returns a generator that randomly constructs one of the commands in `Ts`. If the command has a constructor that matches the (zero or more) arguments given to the callable, that constructor will be used. Otherwise, the command is assumed to have a default constructor. The constructor can make use of the `*`-operator just like in a property or in `gen::exec`.

A common thing to do is to pass the model state as the constructor argument. Here is an example of a command designed for such use:

```C++
struct Take : rc::state::Command<MyModel, MySut> {
  std::string item;
  
  explicit Take(const MyModel &s0)
      : item(*rc::gen::elementOf(s0.items)) {}

  // ...
};
```

Typical usage of `execOneOfWithArgs` is to create the generation function argument for `rc::state::gen::commands` or `rc::state::check`:

```C++
MyModel initialState;
MySut sut;
rc::state::check(initialState,
                 sut,
                 rc::state::gen::execOneOfWithArgs<Put, Take, Frobnicate>());
```

If the model is non-copyable, we can still use this to pass some copyable values of the model state:

```C++
MySut sut;
rc::state::check(
    [] { return MyNonCopyableModel(); },
    sut,
    [](const MyNonCopyableModel &state) {
      return rc::state::gen::execOneOfWithArgs<Put, Take, Frobnicate>()(
          state.status(), state.remaining());
    });
```

With the construction shown above, the commands would take the return types of `state.status()` and `state.remaining()` as parameters to their constructors.

Some notes:
- In the first example, we also have to assert that `item` exists in the model in our `apply` implementation since a generated command might be used for a different state, in particular during shrinking where the state may change because the commands prior in the sequence may be shrunk or removed. For more about this, see the [Stateful testing](state.md) section.
- Discarding macros such as `RC_PRE` and `RC_DISCARD` can be used in the constructor of a command to immediately discard the command if it is not valid for the given state.

#### `Gen<std::shared_ptr<const Command>> gen::execOneOf<Ts...>(Model state)` ####
_**Note**: This function is deprecated, use `rc::gen::execOneOfWithArgs` instead._

Returns a generator that randomly constructs one of the commands in `Ts`. The constructor can make use of the `*`-operator just like in a property or in `gen::exec`. If the command has a constructor taking a model state, `state` will be passed to it on construction. Otherwise, the command is assumed to have a default constructor.

#### `Gen<Commands<Cmd>> gen::commands(Model initialState, GenFunc f)`<br/>`Gen<Commands<Cmd>> gen::commands(MakeModel makeInitialState, GenFunc f)` ####
Generates a valid sequence of commands for the given model state.

`f` should be a function that takes a `Model` as an argument and returns a `Gen<std::shared_ptr<const Command<Model, Sut>>>`. This type looks complicated but it is essentially a generator of `shared_ptr`s to appropriate commands. The generator does not have to yield only valid commands since `RapidCheck` will simply try again if a command is not valid. However, performance will be better if fewer commands have to be discarded, of course. The easiest way to create such a function is to use the `gen::execOneOfWithArgs` function.

There are two overloads of this function, one that takes the state as an immediate value and and another that takes a callable that returns the state.

Since the System Under Test is not specified, the type of the generated commands must be explicitly specified. It cannot be deduced.

## Utilities ##
#### `bool isValidCommand(Command command, const Model &state)` ####
Returns `true` if the given command is valid for the given model state, `false` otherwise.
