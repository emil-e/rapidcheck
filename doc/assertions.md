Assertions
==========
There are multiple ways to fail a test case:

- Returning `false`
- Throwing an unrecognized exception
- Using assertions

Each is equally valid but using RapidCheck's assertions are a convenient way to also provide relevant messages about what failed. Additionally, if to discard tests cases, using the precondition assertions are your only option.

**NOTE:** The assertion macros are implemented using exceptions. The particular exceptions are an implementation detail but you should ensure your properties are exception safe or be prepared to leak resources.

## Capturing ##
In a lot of cases, RapidCheck can capture the expansion of the expression used in the assertion macros in a way inspired by the [Catch.hpp](https://github.com/philsquared/Catch) framework. This means you can write your assertions in a style that is natural to C++:

```C++
std::string foo = "foo";
std::string bar = "bar";
RC_ASSERT(foo == bar);
```

If used in a property, RapidCheck might print something similar:

```
main.cpp:24:
foo == bar

Expands to:
"foo" == "bar"
```

However, in some cases, this capturing might fail to include the information that you want.

## Reference
The selection of assertion macros is currently rather slim. Suggestions on how to improve this is welcome.

### `RC_ASSERT(expression)` ###
Fails the test case if `expression` evaluates to `false`.

### `RC_FAIL(msg)` ###
Unconditionally fails the test case with `msg` as message.

### `RC_SUCCEED_IF(expression)` ###
Causes the test case to immediately succeed if `expression` evaluates to `true`.

### `RC_SUCCEED(msg)` ###
Unconditionally makes the test case succeed with `msg` as the message.

### `RC_PRE(expression)` ###
Discards the test case if `expression` evaluates to `false`.

### `RC_DISCARD(msg)` ###
Unconditionally discards the test case with `msg` as the message.

