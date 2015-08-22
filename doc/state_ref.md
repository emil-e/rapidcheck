# Stateful testing - reference #
All commands that are part of the state testing framework reside under the `rc::state` namespace. To use them, you must include the `rapidcheck/state.h` header. Type signatures are not to be taken literally.

## Testing ##
#### `void check(Model initialState, Sut sut, GenFunc f)` ####
Generates a valid command sequence for `initialState` and `sut` and then runs that sequence on `sut`. Any assertions triggered when running the command sequence will cause the property to fail. For more information about the generation function parameter, see `rc::state::gen::commands`.

This function must be used inside a property, it cannot be used standalone.

## `Command<Model, Sut>` ##
Represents an operation in the state testing framework. The `Model` type parameter is the type of the model that models `Sut` which is the actual System Under Test. These can also be accessed through the `Model` and `Sut` member type alises.

#### `virtual void apply(Model &s0) const` ####
Applies this command to the given state. Preconditions can be asserted using any of the discarding macros such as `RC_PRE` or `RC_DISCARD`. Generation failure (such as trying to generate a value from an empty range using `gen::inRange`) will also cause the commands to be discarded. If the command is discarded, RapidCheck will simply try to generate a new one. This method is intended to be overriden but has a default implementation that does nothing, something that can be useful for commands that do not modify state.

#### `virtual void run(const Model &s0, Sut &sut) const` ####
Applies this command to the given System Under Test. The state before this command has been applied is also passed in. If you need the post state, you can get this using the `nextState` convenience method. This is the method in which to place your assertions. This method is intended to be overriden but has a default implementation that does nothing.

#### `virtual void show(std::ostream &os) const` ####
Outputs a string representation of the command to the given output stream. The default implementation outputs the type name (via RTTI) but if your commands has any sort of parameters, you will likely want to override this with a custom implementation.

#### `Model nextState(const Model &s0) const` ####
Convenience method that calls `apply` on a copy of the given state and returns it. Saves some keystrokes occasionally since you don't have to allocate a model on the stack. Cannot be overridden, override `apply` instead.

## Command sequences ##
#### `Commands<CommandType>` ####
Type alias for an `std::vector<std::shared_ptr<const CommandType>>`. Useful since command sequences are generally stored in vectors and individual commands are usuall passed by `shared_ptr`-to-const.

#### `void applyAll(Commands commands, Model State)` ####
Applies the given commands in sequence to the given model state.

#### `void runAll(Commands commands, Model state, Sut sut)` ####
Runs the given commands in sequence on the given System Under Test assuming the given model state.

#### `bool isValidSequence(Commands commands, Model state)` ####
Returns `true` if the given command sequence is valid for the given model state, otherwise `false`.

## Generators ##
#### `Gen<std::shared_ptr<const Command>> gen::execOneOf<Ts...>(Model state)` ####
Returns a generator that randomly constructs one of the commands in `Ts`. The constructor can make use of the `*`-operator just like in a property or in `gen::exec`. If the command has a constructor taking a model state, `state` will be passed to it on construction. Otherwise, the command is assumed to have a default constructor.

Example of a command with a constructor for use with `execOneOf`:

```C++
struct Take : rc::state::Command<MyModel, MySut> {
  std::string item;
  
  explicit Take(const MyModel &s0)
      : item(*rc::gen::elementOf(s0.items)) {}

  // ...
};
```

Typically, a pointer to an instance of this template will be passed to `rc::state::check`:

```C++
MyModel initialState;
MySut sut;
rc::state::check(initialState,
                 sut,
                 &rc::state::gen::execOneOf<Put, Take, Frobnicate>)
```

However, the template can of course be used as is just as well.

Some notes:
- In the first example, we also have to assert that `item` exists in the model in our `apply` implementation since a generated command might be used for a different state, in particular during shrinking where the state may change because the commands prior in the sequence may be shrunk or removed. For more about this, see the [Stateful testing](state.md) section.
- Discarding macros such as `RC_PRE` and `RC_DISCARD` can be used in the constructor of a command to immediately discard the command if it is not valid for the given state.

#### `Gen<Commands<Cmd>> gen::commands(Model initialState, GenerationFunc f)` ####
Generates a valid sequence of commands for the given model state.

`f` should be a function that takes a `Model` as an argument and returns a `Gen<std::shared_ptr<const Command<Model, Sut>>>`. This type looks complicated but it is essentially a generator of `shared_ptr`s to appropriate commands. The generator does not have to yield only valid commands since `RapidCheck` will simply try again if a command is not valid. However, performance will be better if fewer commands have to be discarded, of course. The easiest way to create such a function is to simply to pass in a pointer to an instance of the `execOneOf` template function.

Since the System Under Test is not specified, the type of the generated commands must be explicitly specified. It cannot be deduced.

## Utilities ##
#### `bool isValidCommand(Command command, Model state)` ####
Returns `true` if the given command is valid for the given model state, `false` otherwise.
