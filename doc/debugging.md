Debugging failures
==================
In a lot of cases, the cause of a property failure is evident in the counterexample given by RapidCheck. However, if this is not the case, RapidCheck provides some tools to let you observe the failure in a debugger.

## Setting breakpoints ##
When RapidCheck finds a failing test case, it shrinks the test case by recursively trying smaller versions of the original test case. It repeats this process until it cannot find a smaller test case which still fails the property. Since this minimal test case is probably the one that you want to debug, RapidCheck makes it easy for you by calling the `rc::beforeMinimalTestCase` function before the minimal test case is run one last time. By setting a breakpoint on this function, you can pause execution to set up your debugging environment (breakpoints, watchpoints etc.) for the final run.

Please note that this is called for every failing property so you may want to use the facilities of a test framework (i.e. Boost UTF or Google Test) to filter out the specific property you want to debug.

## Reproducing a failure ##
RapidCheck randomizes its seed each time the test program is run. This means that if you simply rerun a test program after a failure you might not see the bug again. Fortunately, RapidCheck gives you some control over this randomness so that the exact same failure can be reproduced.

### Reusing seeds ###
RapidCheck prints the exact configuration (including the seed) that it uses when the program starts:

```
Using configuration: seed=12776003016957408636
```

If we rerun the program with the same configuration we will get an identical run:

```
RC_PARAMS="seed=12776003016957408636" ./my_test
```

### Reproduce mode ###
If the bug occured after large number of test cases, waiting for test cases that are known to be successful to run before the failing test case is run can be time consuming. For example, if the bug occured after 2000 tests and 200 shrinks, it is not interesting to run the 1999 (successful) test cases before that and it is not really necessary to try all the shrunk versions of the failure that were tried byt could not reproduce the bug.

Fortunately, RapidCheck will print a string that encodes the necessary information to avoid this to the console if there are failures:

```
Some of your RapidCheck properties had failures. To reproduce these, run with:
RC_PARAMS="reproduce=BMjUhBXakNEalN2aFhXYtBHbl9CZpZXaklmbnJUeUVmbNF2alNXQsxmT11mYlJ3cFFXdhxGZj9A1PXTXZQ2YPQ9z10VGkN2DU_cNdlBZj9A1PXTXZAIgCAAEPAAAAMgADQA"
```

Simply run your test again with this configuration to reproduce the failure:

```
RC_PARAMS="reproduce=BMjUhBXakNEalN2aFhXYtBHbl9CZpZXaklmbnJUeUVmbNF2alNXQsxmT11mYlJ3cFFXdhxGZj9A1PXTXZQ2YPQ9z10VGkN2DU_cNdlBZj9A1PXTXZAIgCAAEPAAAAMgADQA" ./my_test
```

When in reproduce mode, RapidCheck will only run the failing test case and will immediately find the minimal test case without having to try shrinks known to not produce the failure. Properties that did not fail in the original run will be skipped entirely. The `reproduce` parameter alone is sufficient, you do not have to specify `max_size`, `seed` or other test parameters. All required information is encoded in the reproduce string.

While RapidCheck will try to perform as little unnecessary work as possible in reproduce mode, the property might still be run multiple times for implementation reasons. You can break on `rc::beforeMinimalTestCase` as described above to observe the actual minimal run.

*Note:* This feature currently does not work with the Catch.hpp integration.

## Logging ##
Since the property function may be called many times, using classic debugging techniques such as printing to `stdout` or `stderr` can prove confusing. The `RC_LOG` macro provides a better way to log information during the execution of a test case. Information logged this way will only be printed on failure after the minimal test case has been found.

This macro can be use in two ways. If given no arguments, it works like an `std::ostream`:

```C++
RC_LOG() << "Something broke!" << std::endl;
```

When passed a string argument it will instead immediately log the message followed by a newline:

```C++
RC_LOG("It's all broken!");
```

As stated above, none of this will have any effect on the sucess or failure of the test case. The information will only be printed when the test case fails otherwise. For example:

```
Falsifiable after 38 tests and 2 shrinks

std::tuple<User>:
({ username: "%", gender: Female})

../examples/classify/main.cpp:43:
RC_ASSERT(system.login(user) == Result::Success);

Expands to:
Result::InternalError == Result::Success

Log:
Something broke!
```
