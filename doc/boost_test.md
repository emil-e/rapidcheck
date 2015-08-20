Boost Test integration
=======================
RapidCheck comes with support for integrating with Boost Test and allows you to easily create properties that are also Boost Test test cases.

# Usage #
This support is available through the `extras/boost_test` module. You can either directly add the `extras/boost_test/include` directory to your include path or link against the `rapidcheck_boost_test` target in your `CMakeLists.txt`. You can then simply `#include <rapidcheck/boost_test.h>`. Note that `rapidcheck/boost_test.h` needs to be included after any Boost Test headers.

This header defines the `RC_BOOST_PROP` macro which allows you to create RapidCheck properties much in the same way that you would create Boost Test tests:

```C++
RC_BOOST_PROP(copyOfStringIsIdenticalToOriginal,
              (const std::string &str)) {
  const auto strCopy = str;
  RC_ASSERT(strCopy == str);
}
```

This macro takes two arguments, the name of the test case a parenthesized list of arguments that will be generated (just like if you were to use `rc::check` and others). The parenthesis around the argument list are required because of how the preprocessor works and can not be omitted. This also means that if you don't want any arguments generated, you need to include an empty set of paranthesis:

```C++
// If you don't have any arguments, you have to have an empty paren
RC_BOOST_PROP(inRangeValueIsInRange, ()) {
  const auto range = *rc::gen::arbitrary<std::pair<int, int>>();
  const auto x = *rc::gen::inRange(range.first, range.second);
  RC_ASSERT(x >= range.first);
  RC_ASSERT(x < range.second);
}
```

# Assertions #
RapidCheck will treat any exception as a property failure so you should be able to use any assertion mechanism that signals failures as exceptions. However, Boost Test assertions are not implemented using exceptions which means that you should avoid them and use RapidCheck assertions such as `RC_ASSERT` in your RapidCheck properties instead.