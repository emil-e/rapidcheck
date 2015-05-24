Writing properties
==================
To create a property in RapidCheck, you use the `rc::check` function. This function comes in two different forms:

```C++
bool check(Testable);
bool check(std::string, Testable);
```

The only difference between these two is that the second one takes a description description of the promise which will be printed to stderr then the property is run. If the property succeeds, `true` will be returned, otherwise `false` will be returned.

The other argument whose type we denote as `Testable` and this is the actual implementation of the property. This argument is a callable that will be called repeatedly by RapidCheck with different inputs to try and find a set of inputs for which the property does not hold. This callable must meet the following:

- Its parameters must have types that RapidCheck knows how to generate. The arguments must taken by value (`T`), by const reference (`const T &`) or by rvalue reference (`T &&`). Non-const references are not allowed since the arguments are passed as rvalues (temporaries). When called, RapidCheck will generate random values for these parameters. For more information about this, check out the documentation on [generators](generators.md).
- Its return type must be either `void` or `bool`. If the return type is `void`, the property is assumed to succeed unless an exception is thrown, most likely by an [assertion macro](assertions.md). If the property returns `bool`, `true` is taken to mean success while `false` means failure.
- The call operator must not be a template.

The common case is to use a lambda for the callable but any callable object may be used. The following is an example of a property that describes the relationship between the `.size()` and `.empty()` member functions of `std::string`:

```C++
rc::check([](const std::string &str) {
  return str.empty() == (str.size() == 0);
});
```

## Checking preconditions ##
Many properties that we want to check are only valid if certain preconditions on the inputs are met. For example, if we wanted to test a function that splits a string into two equally sized parts, the precondition would be that the length of the string must be even. This can be achieved with the `RC_PRE` macro like this:

```C++
rc::check([](const std::string &str) {
  RC_PRE((str.size() % 2) == 0);
  // ...
});
```

If the precondition does not hold (if the string length is odd), the test case will be discarded and RapidCheck will try again with another set of inputs. Using this functionality is fine if failing preconditions are rare. However, you can run into performance issues (or worse, that Rapidcheck simply [gives up](#gaveup)) if failing preconditions are too frequent. In these cases, you should try to use a generator with much lower chance of generating undesirable data. See the docs on [generators](generators.md) for more info.

## Results ##
There are three different possible outcomes of running a property and each outcome corresponds to the outcome of a single test case invocation.

### Success ###
If enough tests succeed (by default 100 tests but this is [configurable](configuration.md)), RapidCheck will let the property pass and print something similar to:

```
OK, passed 100 tests
```

This, of course, is not a proof that nothing is wrong, just that nothing was found this time around.

### Failure ###
If RapidCheck finds a failing test, it will try to find the smallest test case that still produces a failure. When all possible ways of shrinking the inputs have been exhausted RapidCheck will print something similar to:

```
Falsifiable after 12 tests and 10 shrinks

std::tuple<std::vector<int>>:
([1, 0, 0, 0, 0, 0, 0, 0, 0, 0])

main.cpp:17:
RC_ASSERT(l0 == l1)

Expands to:
[1, 0, 0, 0, 0, 0, 0, 0, 0, 0] == [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
```

This message includes the number of tests that had to be run to find the failure and the number of shrinks that had to be performed to arrive at the inputs that are included in the counterexample. RapidCheck treats arguments as tuples and because of that, the first item in the counterexample is always a tuple. The message also includes the specific condition that made the test case fail, in this case it was an assertion on line 17 in `main.cpp` where the actual value was not equal to the expected one.

### <a name="gaveup"></a>Gave up ###
If enough test cases are discarded, RapidCheck will eventually give up. The threshold after which it gives up is [configurable](configuration.md) but defaults to 10 discards per successful test. If you run 100 tests, this means that a maximum of 1000 test cases may be discarded before RapidCheck gives up. If it does, it will print something similar to:

```
Gave up after 84 tests

../examples/newgen/main.cpp:10:
RC_PRE(l0.size() < 4)

Expands to:
18 < 4
```

The message says that 84 successful tests were performed before RapidCheck gave up trying to run the property. It also displays information about the specific condition which discarded the last test case before giving up.

It is important to note that giving up does not usually indicate a problem with the code that is being tested but rather with the property itself. The problem can be resolved by using more [appropriate generators](generators.md).
