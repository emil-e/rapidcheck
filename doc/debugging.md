Debugging
=========
When RapidCheck finds a failing test case, it shrinks the test case by trying smaller versions of the original test case. It repeats this process until it cannot find a smaller failing case. Since this is probably the test case that you want to debug, RapidCheck makes it easy for you to do this by calling the `rc::beforeMinimalTestCase` function before it runs the minimal test case one last time. By setting a breakpoint on this function, you can pause execution to set up your debugging environment (breakpoints, watchpoints etc.) for the final run.

Please note that this is called for every failing property so you may want to use the facilities of a test framework (i.e. Boost UTF or Google Test) to filter out the specific property you want to debug.

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
