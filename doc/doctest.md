# Doctest integration

rapidcheck comes with a basic integration for the [doctest](https://github.com/doctest/doctest) library.

## Usage

This support is available through the `extras/doctest` module. You can either
add the `extras/doctest/include` directory directly to your include path:

```cmake
add_subdirectory(rapidcheck)
set include_directories(rapidcheck/extras/doctest/include)
add_executable(MyTest main.cpp)
```

...or else link against the `rapidcheck_doctest` cmake target:

```cmake
add_subdirectory(rapidcheck)
add_executable(MyTest main.cpp)
target_link_libraries(MyTest rapidcheck_doctest)
```

Either way, you can then write:

```cpp
#include <doctest/doctest.h>
#include "rapidcheck.h"
#include "rapidcheck/doctest.h'
```

## Reference

### `rc::doctest::check("My test description", []{return true;}, /*verbose=*/true)`

The `rc::doctest::check` function is a drop-in replacement for `rc::check` that reports its success or failure to the `doctest` test runner for inclusion in the statistics gathered for a test run.

The third parameter is optional and defaults to `false`.

```cpp
TEST_CASE("001: My first test case")
{
  rc::doctest::check("integer addition is commutative",
    [](int a, int b)
    {
      return a + b == b + a); // true for success, false for failure
    });

  // no problem mixing rapidcheck tests with other doctest assertions
  SUB_CASE("Normal doctest stuff")
  {
    REQUIRE(1 == 1);
  }
}
```