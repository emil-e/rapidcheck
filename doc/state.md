Stateful testing
=========================
Standard RapidCheck properties are great for testing simple code where RapidCheck generates the input and the property function verifies that this input yields the correct output. This work surprisingly well even for stateful objects such as data structures:

```C++
rc::check("remove(std::string) retrieves the value of a key",
    [](const std::map<std::string, std::string> &data) {
      FastKvStore store(data);
      const auto key = *rc::gen::elementOf(data).first;
      store.remove(key);
      RC_ASSERT(!store.hasKey(key));
    });
```

In this example, we are testing the `remove` method of (presumably) super fast key value store. In the spirit of property based testing, we want to test that this method does the expected thing given arbitrary but valid inputs. But the arguments to `remove` are not the only relevant inputs. If we want to thoroughly test `remove`, we need to randomize the state of `store` in addition to its arguments. Here, we are relying on the fact that we have a simple way to put our system into an arbitrary state by simply constructing it with some arbitrary data. However, we are not always so lucky that there is such a constructor. And even if there is, there might be hidden state that we cannot easily replicate. How can we then randomize the state of a stateful system?

Generally, any valid state state of a system should be reachable by performing some sequence of valid operations on that system. In other words, we should be able to achieve arbitrary valid states by executing arbitrary valid sequences of operations. We can therefore still write our tests in terms of generated input and expected output but for such systems, the input is a valid sequence of operations.

## `rc::state` ##
RapidCheck provides a framework that takes exactly this approach in the `rapidcheck/state.h` header. As developer, you need to provide two things:

- Implementations of the operations that can be performed on the System Under Test.
- A model of the state of the System Under Test.

The operations are represented as subclasses of the `rc::state::Command<Model, Sut>` template and the model is usually just a simple struct containg enough information to model the expected behavior of the system.

The need for a model might require some explanation. First, a system does not always provide enough public information to be able to know what is the expected behavior. Systems typically have lots of hidden state that we cannot see but still want to test. More importantly, the model allows RapidCheck to generate valid sequences of commands without even running the actual System Under Test. This is especially important while shrinking a test case since we do not want to try a shrunk sequence only to find out that it was not valid. This could lead to very slow shrinking.

## A model for `FastKvStore` ##
Since `FastKvStore` is essentially just a map of strings to strings, the model for this class is really simple:

```C++
struct FastKvStoreModel {
  std::map<std::string, std:.string> data;
};
```

In this case, we could have made `std::map<std::string, std::string>` directly be the model but having a struct is nice if you later want to add some more state.

## `Command<Model, Sut>` ##
In RapidCheck, an operation is represented by an object inheriting from the `rc::state::Command<Model, Sut>` interface. The model type parameter is type of the model of the system state. The `Sut` type parameter is the type of the System Under Test. This can either be the type of an actual class that we want to test or it can be a fixture depending on the requirements.

Here is an example implementation of the `remove` operation for our `FastKvStore`:

```C++
struct Remove : rc::state::Command<FastKvStoreModel, FastKvStore> {
  std::string key;
  
  void apply(FastKvStoreModel &s0) const override {
    RC_PRE(s0.data.count(key) != 0);
    s0.data.erase(key);
  }

  void run(const FastKvStoreModel &s0, FastKvStore &sut) const override {
    sut.remove(key);
    RC_ASSERT(!sut.hasKey(key));
  }

  void show(std::ostream &os) const override {
    os << "get(" << key << ")";
  }
};
```

There are two _main_ methods here that you need to implement, `apply` and `run`. `apply` applies the command to the model and `run` applies the command to the actual System Under Test.

`apply` takes a model state by non-const reference and is expected to perform the equivalent of the operation represented by the command on the state. However, if the command is not valid for the given state, you can use any of the RapidCheck discarding macros such as `RC_PRE` and `RC_DISCARD`. RapidCheck will recognize this, discard the command and try another one. In the example above, a precondition for removing a key is that the key actually exists. If it does, we remove it from the model.

`run` applies the same operation but to the System Under Test instead. The expected state before the operation is applied is given as an argument in addition to a reference to the SUT. This is also where you should place your assertions. In the example above, we remove the key from our system (the `FastKvStore`). After this we assert that the SUT no longer contains the key that we removed.

It is also recommended (but not required) to implement the `show` method. This method should output a string representation of what the command does using the given output stream. This representation is used when displaying counterexamples on a test case failure.

## Putting it all together ##
When you have the model and the implementations of the commands, you need to put it all together. The most convenient way to do that is to use the `rc::state::check` function. This function is intended to be used directly in a RapidCheck property, like this:

```C++
#include <rapidcheck/state.h>

//...

rc::check([] {
  FastKvStoreModel initialState;
  FastKvStore sut;
  rc::state::check(initialState,
                   sut,
                   &rc::state::gen::execOneOf<Put, Get, Remove>);
});
```

This function takes three arguments. The first two is the initial model state and a reference to the System Under Test. The initial state must obviously match the state of the System Under Test, otherwise the property will certainly fail due to incorrect expectations.

The third argument should be a callable that takes a model state and returns a command generator appropriate for this state. Specifically, the return type should be `Gen<std::shared_ptr<const Command<Model, Sut>>`, a generator of `std::shared_ptr` to const commands of the appropriate type. The use of `std::shared_ptr` has two reasons:

- We need to erase the specific type of the command so we have a pointer to its base.
- We use `shared_ptr` instead of `unique_ptr` since `shared_ptr` is copyable. Coupled with the constness of the pointee, this gives us value semantics and all the niceness that comes with it.

The returned generator does not have to always generate commands that are valid for the given state. If the returned command is not valid, it will be discarded and a new one will be generated instead.

In the example above, we use a pointer to the `rc::state::gen::execOneOf` template function. This function returns a generator which randomly generates one of its type parameters. For more information about this function, see the [reference documentation](state_ref.md).

Using these parameters, `rc::state::check` will generate a valid command sequence for the given initial state and run it on the System Under Test. If there is an assertion failure when the command sequence is run, the property will fail and a minial counterexample will be printed.

## Command generation ##
The state that is passed to the command generator function in `rc::state::check` (and `rc::state::gen::commands`, see the [reference](state_ref.md)) is always the state that will be used for the generated command. If the state changes during shrinking, a new command will be generated. The nice consequence of this is that if a command is valid for the state for which it was generated, it need not assert any preconditions. This is especially useful for `rc::state::gen::execOneOf`.

## Tips ##
### Generate initialization parameters ###
Since the `rc::state::check` call is part of a regular RapidCheck property, you are free to also generate the initalization data for the model and state. If we for example modify the above example a bit:

```C++
rc::check([](const std::map<std::string, std::string> &data) {
  FastKvStoreModel initialState;
  initialState.data = data;
  FastKvStore sut(data);
  rc::state::check(initialState,
                   sut,
                   &rc::state::gen::execOneOf<Put, Get, Remove>);
});
```
