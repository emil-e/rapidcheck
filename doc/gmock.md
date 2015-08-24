Google Mock integration
=======================
RapidCheck support integration with Google Mock by making it possible for mock failures to make the current RapidCheck test case fail. For RapidCheck, you can most likely not use the `throw_on_failure` flag. This flag causes throws from mock destructors, something which is likely to make the program crash. In traditional unit testing, this might be okay but this prevents RapidCheck from shrinking the test case once it has failed.

## Usage ##
Google Mock integration is not part of RapidCheck core so you need to add `extras/gmock/include` to your include path. You then need to include the `rapidcheck/gmock.h` header and call `rc::gmock::RapidCheckListener::install()` in your main file after Google Mock initialization.

**Example:**

```C++
#include <gmock/gmock.h>

// Here is where Google Mock integration lives
#include <rapidcheck/gmock.h>

int main(int argc, char **argv) {
  ::testing::InitGoogleMock(&argc, argv);

  // This installs the RapidCheck listener.
  rc::gmock::RapidCheckListener::install();

  return RUN_ALL_TESTS();
}
```

This will install a Google Test test event listener which will translate mock failures in to RapidCheck failures.

## Caveats ##
RapidCheck replaces the default test listener with its own and wraps it so that the original listener gets the callbacks as usual when not in a property. However, even when in a property, there is no way for RapidCheck to prevent mock failures and Google Test assertion failures from being reported as errors to Google Test. This is usually not a problem since a single mock failure or assertion failure in a property usually means an actual failure. However, if you see weird behavior when you use Google Test together with RapidCheck, this might be the cause.
