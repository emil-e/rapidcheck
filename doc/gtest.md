Google Test integration
=======================
RapidCheck comes with support for integrating with Google Test and allows you to easily create properties that are also Google Test test cases.

# Usage #
This support is available through the `extras/gtest` module. You can either directly add the `extras/gtest/include` directory to your include path or link against the `rapidcheck_gtest` target in your `CMakeLists.txt`. You can then simply `#include <rapidcheck/gtest.h>`. Note that `rapidcheck/gtest.h` needs to be included after `gtest/gtest.h`.

This header defines the `RC_GTEST_PROP` macro which allows you to create RapidCheck properties much in the same way that you would create Google Test tests:

```C++
RC_GTEST_PROP(MyTestCase,
              copyOfStringIsIdenticalToOriginal,
              (const std::string &str)) {
  const auto strCopy = str;
  RC_ASSERT(strCopy == str);
}
```

This macro takes three arguments, the name of the test case, the name of the test and finally a parenthesized list of arguments that will be generated (just like if you were to use `rc::check` and others). The parenthesis around the argument list are required because of how the preprocessor works and can not be omitted. This also means that if you don't want any arguments generated, you need to include an empty set of paranthesis:

```C++
// If you don't have any arguments, you have to have an empty paren
RC_GTEST_PROP(MyTestCase, inRangeValueIsInRange, ()) {
  const auto range = *rc::gen::arbitrary<std::pair<int, int>>();
  const auto x = *rc::gen::inRange(range.first, range.second);
  RC_ASSERT(x >= range.first);
  RC_ASSERT(x < range.second);
}
```

# Assertions #
RapidCheck will treat any exception as a property failure so you should be able to use any assertion mechanism that signals failures as exceptions. However, Google Test assertions are not implemented using exceptions which means that you should avoid them and use RapidCheck assertions such as `RC_ASSERT` in your RapidCheck properties instead.
